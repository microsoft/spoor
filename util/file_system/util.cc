// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/file_system/util.h"

#include <string>

#include "absl/strings/match.h"
#include "util/env/env.h"

namespace util::file_system {

auto ExpandTilde(std::string path, const env::StdGetEnv& get_env)
    -> std::string {
  if (!absl::StartsWith(path, "~/")) return path;
  const auto home = env::GetEnv(env::kHomeKey, true, get_env).value_or("~");
  path.replace(std::cbegin(path), std::next(std::cbegin(path)), home);
  return path;
}

}  // namespace util::file_system
