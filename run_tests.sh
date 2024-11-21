#!/bin/bash

# Compile the program
clang++ -std=c++11 -Wall *.cc -o lexer
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

passed_count=0
failed_count=0

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
            ((passed_count++))
        else
            echo "$test: FAILED"
            ((failed_count++))
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
        ((failed_count++))
    fi
    echo ""
done
echo "####################################"
echo "Summary:"
echo "Tests Passed: $passed_count"
echo "Tests Failed: $failed_count"
echo "####################################"
