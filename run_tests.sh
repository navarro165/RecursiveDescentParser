#!/bin/bash

# compile program
clang++ -std=c++11 -Wall *.cc -o lexer
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

# Run tests and compare results
echo "####################################"
for test in tests/*.txt; do
    expected="${test}.expected"
    if [ -f "$expected" ]; then
        echo "Running $test..."
        actual_output=$(./lexer < "$test")
        diff_output=$(diff -u <(echo "$actual_output") "$expected")
        if [ $? -eq 0 ]; then
            echo "$test: PASSED"
        else
            echo "$test: FAILED"
            echo "------------------------------------"
            echo "Input:"
            cat "$test"
            echo ""
            echo "Actual Output:"
            echo "$actual_output"
            echo ""
            echo "Expected Output:"
            cat "$expected"
            echo ""
            echo "Diff:"
            echo "$diff_output"
            echo "------------------------------------"
        fi
    else
        echo "$test: No expected output found!"
    fi
    echo ""
done
echo "####################################"
