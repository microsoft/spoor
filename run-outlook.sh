#!/usr/bin/env bash

bazel run //spoor/tools:main -- cat "/Users/lelandjansen/projects/client-cocoa/**/*.spoor_function_map" "/Users/lelandjansen/projects/client-cocoa/trace/*.spoor_trace" > /Users/lelandjansen/Desktop/outlook_trace.json
