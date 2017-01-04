# CTest vs Boost.Test

Boost.Test offers the possibility to execute a number of tests with a single
command invocation and so does CTest. In this document I briefly explain on the
abilities of CTest and Boost.Test as well as a resolution of this issue
(choice).


## CTest Abilities

CTest only run a fixed set of tests. Features:
- run all tests
- run tests matching (not matching) a regular expression
- run tests in random order
- suppress test output by default unless the test fails
- run tests in parallel

There is also CTest Script which may be worth checking out, e.g., for running
Valgrind.


## Boost.Test Abilities

Boost.Test provides C++ facilities for writing, collecting, and grouping tests.
Features:
- [runtime parameters](http://www.boost.org/doc/libs/1_63_0/libs/test/doc/html/boost_test/utf_reference/rt_param_reference.html)
  can be set via command line or environment variables,
- [group tests](http://www.boost.org/doc/libs/1_63_0/libs/test/doc/html/boost_test/tests_organization/tests_grouping.html),
- [run tests conditionally](http://www.boost.org/doc/libs/1_63_0/libs/test/doc/html/boost_test/tests_organization/enabling.html)
- [dependencies between tests](http://www.boost.org/doc/libs/1_63_0/libs/test/doc/html/boost_test/tests_organization/tests_dependencies.html)

An API summary can be found
[here](http://www.boost.org/doc/libs/1_63_0/libs/test/doc/html/boost_test/tests_organization/summary_of_the_api_for_declaring.html).


## Combining Boost.Test with CMake

Seeing that
- Boost tests can be controlled via environment variables,
- CTest allows fine-grained test selection, and
- there is currently no need for dependencies among tests,
Christoph Conrads decided to combine CTest and Boost.test as described in the
blog post
[_Driving Boost.Test with CMake_](https://eb2.co/blog/2015/06/driving-boost-dot-test-with-cmake/)
by Eric Scott Bar. With this approach, all Boost.tests will be registered as
individual CTest tests. Advantages are:
- meaningful CTest output instead of `0 tests failed out of 1`,
- execution of tests in parallel,
- execution of tests in random order,
- fine-grained test selection, and
- full control of Boost.test runtime variables.
Disadvantages are:
- test dependencies are not supported by CTest,
- grouping has to be based purely on test names.
