cmake_minimum_required(VERSION 3.17)
project(project VERSION 1.0.0
  DESCRIPTION project
  LANGUAGES CXX
  ) 
cmake_policy(SET CMP0015 NEW)
include(cmake/Cache.cmake)
set(PROJECT_WARNINGS project_warning)
add_library(${PROJECT_WARNINGS} INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(${PROJECT_WARNINGS})
set(PROJECT_OPTIONS project_options)
add_library(${PROJECT_OPTIONS} INTERFACE)
target_compile_features(${PROJECT_OPTIONS} INTERFACE cxx_std_20)
include(cmake/StaticAnalyzers.cmake) 
include(cmake/Sanitizers.cmake)
enable_sanitizers(${PROJECT_OPTIONS})
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(TARGET_NAME_LIB project_lib)
add_library(${TARGET_NAME_LIB} OBJECT)
add_subdirectory(src) 
target_include_directories(${TARGET_NAME_LIB} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(${TARGET_NAME_LIB} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  )
target_link_libraries(${TARGET_NAME_LIB} 
${PROJECT_WARNINGS}
${PROJECT_OPTIONS}
  CONAN_PKG::matchmaking_proxy
  CONAN_PKG::corrade
)
target_compile_definitions(${TARGET_NAME_LIB} PUBLIC "$<$<CONFIG:DEBUG>:DEBUG>")
target_compile_definitions(${TARGET_NAME_LIB} PUBLIC _LIBCPP_ENABLE_CXX20_REMOVED_TYPE_TRAITS)
# target_compile_definitions(${TARGET_NAME_LIB} PUBLIC BOOST_ASIO_DISABLE_THREADS)
# target_compile_definitions(${TARGET_NAME_LIB} PUBLIC BOOST_ASIO_ENABLE_HANDLER_TRACKING)

# create executable
OPTION(BUILD_TESTS "enable tests" OFF)
IF(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test) 
ENDIF(BUILD_TESTS)
set(TARGET_NAME project)
add_executable(${TARGET_NAME}  
  main.cxx
)
target_link_libraries(${TARGET_NAME} PUBLIC
  ${TARGET_NAME_LIB}
)

target_compile_definitions(CONAN_PKG::matchmaking_proxy INTERFACE PATH_TO_USER_DEFINED_GAME_OPTION="${CMAKE_CURRENT_SOURCE_DIR}/src/userDefinedGameOption.hxx")