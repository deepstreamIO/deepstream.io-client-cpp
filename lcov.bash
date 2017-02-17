#!/bin/bash

lcov --directory . --zerocounters
make test
lcov --directory . --capture --output-file coverage.info
lcov --remove coverage.info 'tests/*' '/usr/*' --output-file coverage.final
genhtml -o coverage.html coverage.final
