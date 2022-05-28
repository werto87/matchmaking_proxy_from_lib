#include "matchmaking_proxy/database/database.hxx"
#include "matchmaking_proxy/logic/matchmakingGame.hxx"
#include "matchmaking_proxy/server/server.hxx"
#include "matchmaking_proxy/userMatchmakingSerialization.hxx"
#include "matchmaking_proxy/util.hxx"
#include <Corrade/Utility/Arguments.h>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <filesystem>
#include <range/v3/algorithm/find_if.hpp>
#include <sodium/core.h>
#ifdef BOOST_ASIO_HAS_CLANG_LIBCXX
#include <experimental/coroutine>
#endif
#include "matchmaking_proxy/server/myWebsocket.hxx"
#include <algorithm> // for max
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/certify/extensions.hpp>
#include <boost/certify/https_verification.hpp>
#include <deque>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator> // for next
#include <openssl/ssl3.h>
#include <range/v3/view.hpp>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility> // for pair

using namespace boost::asio;

int
main (int argc, char *argv[])
{
#ifdef DEBUG
  std::cout << "DEBUG" << std::endl;
#else
  std::cout << "NO DEBUG" << std::endl;
#endif
  auto args = Corrade::Utility::Arguments{};
  // clang-format off
    args
    .addNamedArgument('p', "port").setHelp("port", "port to listen")
    .addNamedArgument('s', "pathToSecrets").setHelp("pathToSecrets", "path to folder with fullchain.pem, privkey.pem and dh2048.pem")
    .setGlobalHelp("A brief description")
    .parse(argc, argv);
  // clang-format on
  try
    {
      if (sodium_init () < 0)
        {
          std::cout << "sodium_init <= 0" << std::endl;
          std::terminate ();
          /* panic! the library couldn't be initialized, it is not safe to use */
        }
#ifdef DEBUG
      database::createEmptyDatabase ();
#else
      database::createDatabaseIfNotExist ();
#endif
      database::createTables ();

      using namespace boost::asio;
      io_context io_context (1);
      signal_set signals (io_context, SIGINT, SIGTERM);
      signals.async_wait ([&] (auto, auto) { io_context.stop (); });
      thread_pool pool{ 2 };
      auto server = Server{ io_context, pool };
      auto const port = boost::lexical_cast<u_int16_t> (args.value ("port"));
      auto const userPort = 55555;
      auto const gamePort = 33333;
      auto const pathToSecrets = std::filesystem::path{ args.value ("pathToSecrets") };
      auto userEndpoint = boost::asio::ip::tcp::endpoint{ ip::tcp::v4 (), userPort };
      auto gameEndpoint = boost::asio::ip::tcp::endpoint{ ip::tcp::v4 (), gamePort };
      using namespace boost::asio::experimental::awaitable_operators;
      co_spawn (io_context, server.userMatchmaking (userEndpoint, pathToSecrets) || server.gameMatchmaking (gameEndpoint), printException);
      io_context.run ();
    }
  catch (std::exception &e)
    {
      std::printf ("Exception: %s\n", e.what ());
    }
  return 0;
  // TODO fix conan recipe so we do not have undifined references
}
