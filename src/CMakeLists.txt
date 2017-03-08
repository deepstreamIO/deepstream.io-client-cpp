set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(OpenSSL REQUIRED)

flex_target(lexer
  ${CMAKE_SOURCE_DIR}/src/lexer.l
  ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)

set_source_files_properties(
	"${CMAKE_CURRENT_BINARY_DIR}/lexer.c"
	PROPERTIES
		COMPILE_DEFINITIONS "_POSIX_SOURCE"
		COMPILE_FLAGS "-Wno-unused-function -Wno-unused-parameter -Wno-type-limits -Wno-sign-compare"
)

add_library(
	libdeepstream SHARED
	client.cpp
	deepstream.cpp
	error_handler.cpp
	event.cpp
	exception.cpp
	impl.cpp
	message.cpp
	message_builder.cpp
	message_proxy.cpp
	parser.cpp
	presence.cpp
	random.cpp
	websockets.cpp
	websockets/poco.cpp
	websockets/pseudo.cpp
	${FLEX_lexer_OUTPUTS})

if(BUILD_POCO)
  set(Poco_LIBRARIES PocoNetSSL PocoCrypto PocoNet PocoUtil PocoJSON PocoXML PocoFoundation)
endif()

set_target_properties(libdeepstream PROPERTIES OUTPUT_NAME deepstream)
target_include_directories(libdeepstream PUBLIC "${CMAKE_BINARY_DIR}/include")
target_link_libraries(libdeepstream PUBLIC ${Poco_LIBRARIES} ${OPENSSL_LIBRARIES} Threads::Threads)
install(TARGETS libdeepstream DESTINATION "lib")

# Ensure parallel builds operate correctly.
if(BUILD_POCO)
  add_dependencies(libdeepstream poco)
endif()
