find_package(Boost 1.46 REQUIRED)
find_package(FLEX 2.5 REQUIRED)


add_custom_command(
	OUTPUT
		"${CMAKE_CURRENT_BINARY_DIR}/lexer.c"
		"${CMAKE_CURRENT_BINARY_DIR}/lexer.h"
	DEPENDS lexer.l
	COMMAND ${FLEX_EXECUTABLE}
		--outfile=lexer.c
		--header-file=lexer.h
		-- "${CMAKE_CURRENT_SOURCE_DIR}/lexer.l"
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
	deepstream_parser SHARED
	message.cpp
	message_builder.cpp
	parser.cpp
	"${CMAKE_CURRENT_BINARY_DIR}/lexer.c")

target_include_directories(deepstream_parser PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(deepstream_parser PUBLIC ${Boost_INCLUDE_DIRS})


add_library(
	deepstream_client SHARED
	client.cpp)

target_link_libraries(deepstream_client PUBLIC deepstream_parser)
