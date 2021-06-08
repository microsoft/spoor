#!/bin/bash

set -euo pipefail

archive_root_path=${1}
framework_path=$(find ${archive_root_path} -type d -name '*.framework')
framework_name=$(basename ${framework_path%.framework*})

plutil -convert binary1 -o "${framework_path}/Info.plist" - <<EOF
{
    "CFBundleExecutable": "${framework_name}",
    "CFBundlePackageType": "FMWK",
    "CFBundleShortVersionString": "0.0.1",
    "MinimumOSVersion": "13.0",
    "CFBundleSupportedPlatforms": [
        "iPhoneOS",
    ],
}
