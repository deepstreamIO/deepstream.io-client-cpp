set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(OpenSSL REQUIRED)

add_custom_command(
	OUTPUT
		${CMAKE_CURRENT_BINARY_DIR}/lexer.c
		${CMAKE_CURRENT_BINARY_DIR}/lexer.h
	DEPENDS ${CMAKE_SOURCE_DIR}/src/lexer.l
	COMMAND ${FLEX_EXECUTABLE}
		--outfile=${CMAKE_CURRENT_BINARY_DIR}/lexer.c
		--header-file=${CMAKE_CURRENT_BINARY_DIR}/lexer.h
		-- ${CMAKE_SOURCE_DIR}/src/lexer.l
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

if(BUILD_POCO)
  set(Poco_LIBRARIES PocoNetSSL PocoCrypto PocoNet PocoUtil PocoJSON PocoXML PocoFoundation)
endif()

set_target_properties(libdeepstream PROPERTIES OUTPUT_NAME deepstream)
target_include_directories(libdeepstream PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(libdeepstream PUBLIC ${Poco_LIBRARIES} ${OPENSSL_LIBRARIES} Threads::Threads)
install(TARGETS libdeepstream DESTINATION "lib")

# Ensure parallel builds operate correctly.
if(BUILD_POCO)
  add_dependencies(libdeepstream poco)
endif()
