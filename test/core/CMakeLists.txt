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

add_custom_command(
    OUTPUT
        lexer.c
        lexer.h
    DEPENDS ${CMAKE_SOURCE_DIR}/src/core/lexer.l
    COMMAND ${FLEX_EXECUTABLE}
        --outfile=lexer.c
        --header-file=lexer.h
        -- ${CMAKE_SOURCE_DIR}/src/core/lexer.l
    COMMENT "[FLEX][src] Building lexer with Flex ${FLEX_VERSION}"
    VERBATIM
)

set_source_files_properties(
    "${CMAKE_CURRENT_BINARY_DIR}/lexer.c"
    PROPERTIES
        COMPILE_DEFINITIONS "_POSIX_SOURCE"
        COMPILE_FLAGS "-Wno-unused-function -Wno-unused-parameter -Wno-type-limits -Wno-sign-compare"
)

include_directories(${CMAKE_SOURCE_DIR})

add_library(test_lexer ${CMAKE_CURRENT_BINARY_DIR}/lexer.c)
target_include_directories(test_lexer PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_compile_definitions(test_lexer PUBLIC -DDEEPSTREAM_TEST_LEXER)

add_library(libdeepstream_core_test SHARED ../dummy.cpp)
set_target_properties(libdeepstream_core_test PROPERTIES OUTPUT_NAME deepstream-core-test)
target_link_libraries(libdeepstream_core_test PUBLIC libdeepstream_core)
install(TARGETS libdeepstream_core_test DESTINATION "lib")

add_boost_test(test-lexer.cpp test_lexer)

add_boost_test(test-connection.cpp libdeepstream_core_test)
add_boost_test(test-event.cpp libdeepstream_core_test)
add_boost_test(test-message.cpp libdeepstream_core_test)
add_boost_test(test-message_builder.cpp libdeepstream_core_test)
add_boost_test(test-parser.cpp libdeepstream_core_test)
add_boost_test(test-presence.cpp libdeepstream_core_test)
add_boost_test(test-random.cpp libdeepstream_core_test)
