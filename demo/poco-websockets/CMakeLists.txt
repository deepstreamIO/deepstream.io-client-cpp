cmake_minimum_required(VERSION 2.8.12)
project(POCO_websockets_spike)
enable_language(CXX)
set(POCO_websockets_spike_VERSION_MAJOR 1)
set(POCO_websockets_spike_VERSION_MINOR 0)
set(POCO_websockets_spike_VERSION_PATCH 0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -std=c11 -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -std=c++11 -pedantic")

if(NOT DEFINED(CMAKE_BUILD_TYPE) OR "Debug" STREQUAL "${CMAKE_BUILD_TYPE}")
	if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_ASSERTIONS")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG_PEDANTIC")
	endif()
endif()


find_package(POCO REQUIRED)


add_executable(poco-ws-client client.cpp)
target_include_directories(poco-ws-client PUBLIC ${POCO_INCLUDE_DIRS})
target_link_libraries(poco-ws-client PUBLIC ${POCO_LIBRARIES})
