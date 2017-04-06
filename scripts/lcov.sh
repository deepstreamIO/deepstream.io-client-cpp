#!/bin/bash

lcov --directory . --zerocounters
make test
lcov --directory . --capture --output-file coverage.info
lcov --remove coverage.info 'test/*' '/usr/*' --output-file coverage.final
mv coverage.final coverage.info
genhtml -o coveragereport coverage.info
