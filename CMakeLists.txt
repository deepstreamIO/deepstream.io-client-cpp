cmake_minimum_required(VERSION 2.8)
project(deepstream.io-client-c++)
enable_language(C)
enable_language(CXX)
set(deepstream.io-client-c++_VERSION_MAJOR 0)
set(deepstream.io-client-c++_VERSION_MINOR 1)
set(deepstream.io-client-c++_VERSION_PATCH 0)


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

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)
