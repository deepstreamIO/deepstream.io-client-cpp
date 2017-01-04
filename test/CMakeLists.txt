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


add_boost_test(simple.cpp deepstream_parser)
