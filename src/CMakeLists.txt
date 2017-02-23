set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(OpenSSL REQUIRED)

add_custom_command(
	OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/lexer.c
		${CMAKE_BINARY_DIR}/include/lexer.h
	DEPENDS lexer.l
	COMMAND ${FLEX_EXECUTABLE}
		--outfile=${CMAKE_CURRENT_BINARY_DIR}/lexer.c
		--header-file=${CMAKE_BINARY_DIR}/include/lexer.h
		-- ${CMAKE_CURRENT_SOURCE_DIR}/lexer.l
	COMMENT "[FLEX][src] Building lexer with Flex ${FLEX_VERSION}"
	VERBATIM
)

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
	"${CMAKE_CURRENT_BINARY_DIR}/lexer.c")

set(POCO_LIBS PocoNetSSL PocoCrypto PocoNet PocoUtil PocoJSON PocoXML PocoFoundation)

set_target_properties(libdeepstream PROPERTIES OUTPUT_NAME deepstream)
target_include_directories(libdeepstream PUBLIC "${CMAKE_BINARY_DIR}/include")
target_link_libraries(libdeepstream PUBLIC ${POCO_LIBS} ${OPENSSL_LIBRARIES} Threads::Threads)
install(TARGETS libdeepstream DESTINATION "lib")


add_executable(ds-example ds-example.cpp)
target_link_libraries(ds-example PUBLIC libdeepstream)

add_executable(ds-client ds-client.cpp)
target_link_libraries(ds-client PUBLIC libdeepstream)

add_executable(fuzz-me fuzz-me.cpp)
target_link_libraries(fuzz-me PUBLIC libdeepstream)

add_executable(make-fuzzer-input make-fuzzer-input.cpp)
target_link_libraries(make-fuzzer-input PUBLIC libdeepstream)

# Ensure parallel builds operate correctly.
add_dependencies(libdeepstream poco)
