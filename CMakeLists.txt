cmake_minimum_required(VERSION 3.2.0)
project(deepstream.io-client-c++ VERSION 0.1.0 LANGUAGES C CXX)


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra -Wall -std=c11 -pedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -std=c++11 -pedantic")


include_directories("include")

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)
