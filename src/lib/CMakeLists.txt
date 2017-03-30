set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

add_library(
  libdeepstream_poco SHARED
  poco-ws.cpp)

if(BUILD_POCO)
  set(Poco_LIBRARIES PocoNetSSL PocoCrypto PocoNet PocoUtil PocoJSON PocoXML PocoFoundation)
endif()

set_target_properties(libdeepstream_poco PROPERTIES OUTPUT_NAME deepstream-poco)
target_include_directories(libdeepstream_poco PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(libdeepstream_poco PUBLIC libdeepstream_core ${Poco_LIBRARIES} ${OPENSSL_LIBRARIES} Threads::Threads)
install(TARGETS libdeepstream_poco DESTINATION "lib")

if(BUILD_POCO)
  add_dependencies(libdeepstream_poco poco)
endif()
add_dependencies(libdeepstream_poco libdeepstream_core)
