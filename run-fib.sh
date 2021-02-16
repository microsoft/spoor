#!/usr/bin/env bash
bazel run //spoor/tools:main -- cat "/Users/lelandjansen/Desktop/MyAmazingApp/**/*.spoor_function_map" "/Users/lelandjansen/Desktop/MyAmazingApp/trace/*.spoor_trace" > ~/Desktop/fib_trace.json
