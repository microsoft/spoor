// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

#include "util/env/env.h"
#include "util/numeric.h"

namespace util::file_system {

struct PathExpansionOptions {
  env::StdGetEnv get_env;
  bool expand_tilde;
  bool expand_environment_variables;
};

auto ExpandPath(std::string path, const PathExpansionOptions& options)
    -> std::string;

}  // namespace util::file_system
