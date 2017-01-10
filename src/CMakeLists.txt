find_package(FLEX REQUIRED)

add_custom_command(
	OUTPUT
		"${CMAKE_CURRENT_BINARY_DIR}/lexer.c"
		"${CMAKE_CURRENT_BINARY_DIR}/lexer.h"
	DEPENDS lexer.l
	COMMAND ${FLEX_EXECUTABLE}
		--outfile=lexer.c
		--header-file=lexer.h
		-- "${CMAKE_CURRENT_SOURCE_DIR}/lexer.l"
	COMMENT "[FLEX][src] Bulding lexer with Flex ${FLEX_VERSION}"
	VERBATIM
)


set_source_files_properties(
	"${CMAKE_CURRENT_BINARY_DIR}/lexer.c"
	PROPERTIES
		COMPILE_DEFINITIONS "_POSIX_SOURCE"
		COMPILE_FLAGS "-Wno-unused-function -Wno-unused-parameter -Wno-type-limits -Wno-sign-compare"
)

find_package(Boost 1.46 REQUIRED)

add_library(
	deepstream_parser SHARED
	message.cpp
	parser.cpp
	"${CMAKE_CURRENT_BINARY_DIR}/lexer.c")

target_include_directories(deepstream_parser PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(deepstream_parser PUBLIC ${Boost_INCLUDE_DIRS})
