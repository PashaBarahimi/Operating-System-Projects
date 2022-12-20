#!/bin/env bash

test_count=10
input="input.bmp"

cd ./serial
make > /dev/null
serial=$(./ImageFilters.out $input $test_count 1)

cd ..

cd parallel
make > /dev/null
parallel=$(./ImageFilters.out $input $test_count 1)

speedup=$(bc -l <<< "$serial / $parallel")
echo "Speedup = ${speedup::6}"

