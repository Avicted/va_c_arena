#!/bin/bash

# set -xe

[ -d ./build ] && rm -rf ./build
mkdir -p ./build

CC=clang

# Note: Using C11 standard for compatibility with the tests that use `alignof`.
CFLAGS="-std=c11 -ggdb -O0 -Wall -Wextra -Werror -Wpedantic"
LDLIBS="-lm"
INCLUDE_DIRS="-I./include"

# Build all targets
$CC $CFLAGS $INCLUDE_DIRS ./src/example.c -o ./build/example $LDLIBS
$CC $CFLAGS $INCLUDE_DIRS ./src/example_aligned.c -o ./build/example_aligned $LDLIBS
$CC $CFLAGS $INCLUDE_DIRS ./tests/test_va_arena.c -o ./build/tests $LDLIBS

# Flags
if [[ "$1" == "example" ]]; then
    ./build/example
elif [[ "$1" == "aligned" ]]; then
    ./build/example_aligned
elif [[ "$1" == "tests" ]]; then
    ./build/tests
elif [[ "$1" == "valgrind" ]]; then
    echo "Valgrind HEAP SUMMARY for example:"
    valgrind -s --leak-check=full ./build/example 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"
    echo
    echo "Valgrind HEAP SUMMARY for example_aligned:"
    valgrind -s --leak-check=full ./build/example_aligned 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"
    echo
    echo "Valgrind HEAP SUMMARY for tests:"
    valgrind -s --leak-check=full ./build/tests 2>&1 | grep -A6 "HEAP SUMMARY\|ERROR SUMMARY"
    exit 0
elif [[ "$1" == "test-stds" ]]; then
    for std in c99 c11 c17 c23; do
        echo "Testing with -std=$std"
        
        $CC -std=$std -ggdb -O0 -Wall -Wextra -Werror -Wpedantic $INCLUDE_DIRS ./src/example_aligned.c -o ./build/example_aligned $LDLIBS

        if [[ $? -eq 0 ]]; then
            ./build/example_aligned
            echo "Build and run successful for -std=$std"
        else
            echo "ERROR: Build failed for -std=$std"
        fi
        echo "--------------------------------------------"
        echo
    done
    exit 0
else
    echo "Build complete. Run with:"
    echo "  ./build.sh example        # Run example"
    echo "  ./build.sh aligned        # Run example_aligned"
    echo "  ./build.sh tests          # Run tests"
    echo "  ./build.sh valgrind       # Run Valgrind to check for memory leaks"
    echo "  ./build.sh test-stds      # Test with different C standards (c99, c11, c17, c23)"
fi

