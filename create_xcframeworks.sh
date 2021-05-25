#!/bin/bash

regex='bazel-out.*zip'

rm -r .xcframework-tmp > /dev/null 2>&1
rm -r xcframeworks > /dev/null 2>&1
mkdir .xcframework-tmp
mkdir .xcframework-tmp/runtime_framework
mkdir .xcframework-tmp/runtime_stub_framework
mkdir .xcframework-tmp/runtime_framework/x86_64
mkdir .xcframework-tmp/runtime_framework/arm64
mkdir .xcframework-tmp/runtime_stub_framework/x86_64
mkdir .xcframework-tmp/runtime_stub_framework/arm64

function create_xcframework {
    name=$1

    echo "$name: Building framework for x86_64..."
    x86_64_bazel_output=$((bazel build //spoor/runtime/wrappers/objc:$name --ios_multi_cpus=x86_64) 2>&1)
    if [ $? -ne 0 ]; then
        echo "$name: Failed to build for x86_64" >&2
        exit $?
    fi
    [[ $x86_64_bazel_output =~ $regex ]]
    x86_64_zip_framework=${BASH_REMATCH[0]}

    echo "$name: Building framework for arm64..."
    arm64_bazel_output=$((bazel build //spoor/runtime/wrappers/objc:$name --ios_multi_cpus=arm64) 2>&1)
    if [ $? -ne 0 ]; then
        echo "$name: Failed to build for arm64" >&2
        exit $?
    fi
    [[ $arm64_bazel_output =~ $regex ]]
    arm64_zip_framework=${BASH_REMATCH[0]}

    unzip $x86_64_zip_framework -d .xcframework-tmp/$name/x86_64 > /dev/null 2>&1
    unzip $arm64_zip_framework -d .xcframework-tmp/$name/arm64 > /dev/null 2>&1

    echo "$name: Creating xcframework..."
    xcodebuild -create-xcframework \
        -framework .xcframework-tmp/$name/x86_64/SpoorRuntime.framework \
        -framework .xcframework-tmp/$name/arm64/SpoorRuntime.framework \
        -output xcframeworks/$name/SpoorRuntime.xcframework
}

create_xcframework runtime_framework
create_xcframework runtime_stub_framework

rm -r .xcframework-tmp