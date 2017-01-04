find_package(FLEX)

FLEX_TARGET(
	lexer
	lexer.l
	"${CMAKE_CURRENT_BINARY_DIR}/lexer.c"
	DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/lexer.h")


set_source_files_properties(
	"${CMAKE_CURRENT_BINARY_DIR}/lexer.c"
	PROPERTIES COMPILE_FLAGS "-Wno-unused-function -Wno-unused-parameter")

add_library(
	deepstream_parser SHARED
	message.cpp
	parser.cpp
	${FLEX_lexer_OUTPUTS})

target_include_directories(deepstream_parser PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
