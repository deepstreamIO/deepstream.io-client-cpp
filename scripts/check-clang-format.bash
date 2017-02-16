#!/bin/bash

# This script ensures that C++/C source files adhere to the projects
# coding style which is based on the 'webkit' style. It exits with 0
# (SUCCESS) if no differences were found, otherwise 1 (FAILURE).

status=0

type -p clang-format > /dev/null 2>&1 || { echo "error: clang-format is not installed"; exit 1; }

[ "${VERBOSE:-0}" = "1" ] && echo "verifying coding style with clang-format..."

for i in $(find * -name '*.cpp' -or -name '*.hpp' -or -name '*.h' -or -name '*.c'); do
    [[ "$i" =~ "lexer" ]] && continue
    [[ "$i" =~ "CMakeFiles" ]] && continue
    [[ "${VERBOSE:-0}" = "1" ]] && echo "verifying coding style on $i"
    diff -u <(cat $i) <(clang-format --style=file $i) > /dev/null
    if [ $? -ne 0 ]; then
        echo "error: clang-format is sad: $i is not compliant." >&2
        status=1
    fi
done

exit $status
