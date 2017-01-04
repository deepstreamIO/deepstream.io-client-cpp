find_package(FLEX)

FLEX_TARGET(
	lexer
	lexer.y
	lexer.c
	COMPILE_FLAGS "-Wno-unused-function -Wno-unused-parameter")

#add_custom_command(
#	OUTPUT lexer.c lexer.h
#	COMMAND ${FLEX} --fast --outfile=lexer.c --header-file=lexer.h lexer.y
#	DEPENDS lexer.y
#	COMMENT "Building Flex lexer")



add_library(
	deepstream_parser
	message.cpp
	parser.cpp
	${FLEX_lexer_OUTPUT})

target_include_directories(deepstream_parser PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
