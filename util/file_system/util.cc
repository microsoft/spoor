// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/file_system/util.h"

#include <string>

#include "absl/strings/match.h"
#include "util/env/env.h"

namespace util::file_system {

using std::literals::string_literals::operator""s;

auto ExpandTilde(std::string path, const env::GetEnv& get_env) -> std::string {
  if (!absl::StartsWith(path, "~/")) return path;
  path.replace(std::cbegin(path), std::next(std::cbegin(path)),
               env::GetEnvOrDefault(env::kHomeKey.data(), "~"s, get_env));
  return path;
}

}  // namespace util::file_system
