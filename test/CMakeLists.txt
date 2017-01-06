if(NOT BUILD_TESTING)
	return()
endif()


# This functions searches all unit tests implemented with boost.test and turns
# each of them into a single CTest test case. See `doc/ctest-vs-boost.md` for
# the motivation behind this.
# Source:
# https://eb2.co/blog/2015/06/driving-boost-dot-test-with-cmake/
function(add_boost_test FILENAME DEPENDENCY_LIB)
	get_filename_component(TEST_EXECUTABLE ${FILENAME} NAME_WE)
	add_executable(${TEST_EXECUTABLE} ${FILENAME})
	target_compile_definitions(${TEST_EXECUTABLE} PUBLIC -DBOOST_TEST_DYN_LINK)
	target_link_libraries(${TEST_EXECUTABLE} ${DEPENDENCY_LIB})
	target_link_libraries(${TEST_EXECUTABLE} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})

	file(READ "${FILENAME}" SOURCE)
	string(
		REGEX MATCHALL
		"BOOST_AUTO_TEST_CASE\\( *([A-Za-z0-9_]+) *\\)"
		TESTS
		${SOURCE})

	foreach(TEST ${TESTS})
		string(
			REGEX REPLACE
			".*\\( *([A-Za-z0-9_]+) *\\).*" "\\1"
			TEST_NAME
			${TEST})

		add_test(
			NAME "${TEST_EXECUTABLE}.${TEST_NAME}"
			COMMAND ${TEST_EXECUTABLE} --run_test=${TEST_NAME})
	endforeach()
endfunction()


find_package(Boost 1.46 REQUIRED COMPONENTS unit_test_framework)
find_package(FLEX REQUIRED)


include_directories(${CMAKE_CURRENT_BINARY_DIR})


# lexer testing
FLEX_TARGET(
	test_lexer
	"${CMAKE_SOURCE_DIR}/src/lexer.l"
	"${CMAKE_CURRENT_BINARY_DIR}/test_lexer.c"
	DEFINES_FILE "${CMAKE_CURRENT_BINARY_DIR}/test_lexer.h")

set_source_files_properties(
	"${CMAKE_CURRENT_BINARY_DIR}/test_lexer.c"
	PROPERTIES COMPILE_FLAGS "-Wno-unused-function -Wno-unused-parameter")

add_library(test_lexer ${FLEX_test_lexer_OUTPUTS})
target_compile_definitions(test_lexer PUBLIC -DDEEPSTREAM_TEST_LEXER)

add_boost_test(message.cpp deepstream_parser)
add_boost_test(lexer.cpp test_lexer)
