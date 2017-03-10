set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

add_library(
	libdeepstream SHARED
	websockets/poco.cpp
	websockets/pseudo.cpp)

if(BUILD_POCO)
  set(Poco_LIBRARIES PocoNetSSL PocoCrypto PocoNet PocoUtil PocoJSON PocoXML PocoFoundation)
endif()

set_target_properties(libdeepstream PROPERTIES OUTPUT_NAME deepstream)
target_include_directories(libdeepstream PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(libdeepstream PUBLIC libdeepstream_core ${Poco_LIBRARIES} ${OPENSSL_LIBRARIES} Threads::Threads)
install(TARGETS libdeepstream DESTINATION "lib")

# Ensure parallel builds operate correctly.
if(BUILD_POCO)
  add_dependencies(libdeepstream poco)
endif()
