// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

#include "util/env/env.h"
#include "util/flat_map/flat_map.h"
#include "util/numeric.h"

namespace util::file_system {

struct PathExpansionOptions {
  bool expand_tilde;
  bool expand_environment_variables;
};

auto ExpandPath(std::string path, PathExpansionOptions options,
                const env::StdGetEnv& get_env) -> std::string;

}  // namespace util::file_system
