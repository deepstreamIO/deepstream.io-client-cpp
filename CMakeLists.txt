cmake_minimum_required(VERSION 2.8.12)

project(deepstream.io-client-c++)

enable_language(C)
enable_language(CXX)
include(CTest)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

# Put the libaries and binaries that get built into directories at the
# top of the build tree rather than in hard-to-find leaf directories.

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(CCACHE "Use ccache if available" ON)
find_program(CCACHE_PROGRAM ccache)
if(CCACHE AND CCACHE_PROGRAM)
  message(STATUS "using ccache")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()

# Use ld.gold if it is available and isn't explicitly disabled.
option(USE_LD_GOLD "Use GNU gold linker" ON)
if(USE_LD_GOLD AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -fuse-ld=gold -Wl,--version OUTPUT_VARIABLE stdout ERROR_QUIET)
  if("${stdout}" MATCHES "GNU gold")
    message(STATUS "matched GNU gold")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-ld=gold")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
  else()
    message(WARNING "GNU gold linker isn't available, using the default system linker.")
  endif()
endif()

option(BUILD_TESTING "Build tests" OFF)

if(BUILD_TESTING)
	enable_testing()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -std=c11 -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -std=c++11 -pedantic")

if(NOT DEFINED(CMAKE_BUILD_TYPE) OR "Debug" STREQUAL "${CMAKE_BUILD_TYPE}")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_ASSERTIONS")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG_PEDANTIC")
	endif()
endif()


include_directories("include")

add_subdirectory(doc)
add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)
