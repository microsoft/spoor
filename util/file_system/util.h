// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

#include "util/env/env.h"
#include "util/flat_map/flat_map.h"

namespace util::file_system {

auto ExpandTilde(std::string path, const env::StdGetEnv& get_env)
    -> std::string;

}  // namespace util::file_system
