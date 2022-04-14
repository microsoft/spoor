# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

RUNTIME_TARGET_NAME = "SpoorRuntime"

RUNTIME_STUB_TARGET_NAME = "SpoorRuntimeStub"

RUNTIME_HDRS = [
    "SpoorConfig.h",
    "SpoorDeletedFilesInfo.h",
    "Runtime.h",
    "SpoorTypes.h",
]

def _runtime_framework_files(framework_name):
    headers = [
        "Headers/{}".format(header)
        for header in RUNTIME_HDRS + ["{}.h".format(framework_name)]
    ]
    modules = ["Modules/module.modulemap"]
    info_plists = ["Info.plist"]
    executables = [framework_name]
    framework_files = headers + modules + info_plists + executables
    return [
        "{}.framework/{}".format(framework_name, file)
        for file in framework_files
    ]

def _runtime_xcframework_files(xcframework_name):
    framework_files = _runtime_framework_files(xcframework_name)
    xcframework_files = ["Info.plist"]
    for variant in ["ios-arm64", "ios-arm64_x86_64-simulator"]:
        xcframework_files += [
            "{}/{}".format(variant, file)
            for file in framework_files
        ]
    return [
        "{}.xcframework/{}".format(xcframework_name, file)
        for file in xcframework_files
    ]

RUNTIME_XCFRAMEWORK_FILES = _runtime_xcframework_files(RUNTIME_TARGET_NAME)

RUNTIME_STUB_XCFRAMEWORK_FILES = _runtime_xcframework_files(
    RUNTIME_STUB_TARGET_NAME,
)
