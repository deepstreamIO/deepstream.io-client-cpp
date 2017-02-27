# This functions searches all unit tests implemented with boost.test and turns
# each of them into a single CTest test case. See `doc/ctest-vs-boost.md` for
# the motivation behind this.
# Source:
# https://eb2.co/blog/2015/06/driving-boost-dot-test-with-cmake/
function(add_boost_test FILENAME DEPENDENCY_LIB)
	get_filename_component(TEST_EXECUTABLE ${FILENAME} NAME_WE)
	add_executable(${TEST_EXECUTABLE} ${FILENAME})
	target_compile_definitions(${TEST_EXECUTABLE} PUBLIC -DBOOST_TEST_DYN_LINK)
	target_include_directories(${TEST_EXECUTABLE} PUBLIC ${Boost_INCLUDE_DIRS})
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


include_directories("${CMAKE_BINARY_DIR}/include")


# lexer testing
add_custom_command(
	OUTPUT
	${CMAKE_CURRENT_BINARY_DIR}/test_lexer.c
	${CMAKE_BINARY_DIR}/include/test_lexer.h
	DEPENDS ${CMAKE_SOURCE_DIR}/src/lexer.l
	COMMAND ${FLEX_EXECUTABLE}
		--outfile=${CMAKE_CURRENT_BINARY_DIR}/test_lexer.c
		--header-file=${CMAKE_BINARY_DIR}/include/test_lexer.h
		-- ${CMAKE_SOURCE_DIR}/src/lexer.l
	COMMENT "[FLEX][test] Building lexer with Flex ${FLEX_VERSION}"
	VERBATIM
)

set_source_files_properties(
	"${CMAKE_CURRENT_BINARY_DIR}/test_lexer.c"
	PROPERTIES
		COMPILE_DEFINITIONS "_POSIX_SOURCE"
		COMPILE_FLAGS "-Wno-unused-function -Wno-unused-parameter -Wno-type-limits -Wno-sign-compare"
)

add_library(test_lexer test_lexer.c)
target_compile_definitions(test_lexer PUBLIC -DDEEPSTREAM_TEST_LEXER)

add_boost_test(lexer.cpp test_lexer)

add_boost_test(client.cpp libdeepstream)
add_boost_test(impl.cpp libdeepstream)
add_boost_test(event.cpp libdeepstream)
add_boost_test(message.cpp libdeepstream)
add_boost_test(message_builder.cpp libdeepstream)
add_boost_test(parser.cpp libdeepstream)
add_boost_test(presence.cpp libdeepstream)
add_boost_test(random.cpp libdeepstream)

if(SWIG_FOUND)
  add_test(
    NAME    python-swig-test
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/python-swig-test.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
