// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/file_system/util.h"

#include <string>

#include "absl/strings/match.h"
#include "util/env/env.h"

namespace util::file_system {

auto ExpandTilde(std::string path, const env::StdGetEnv& get_env)
    -> std::string {
  using Result = util::result::Result<std::string, util::result::None>;
  if (!absl::StartsWith(path, "~/")) return path;
  const auto result =
      env::GetEnv(env::kHomeKey, true, get_env).value_or(Result::Ok("~"));
  if (result.IsOk()) {
    const auto& home = result.Ok();
    path.replace(std::cbegin(path), std::next(std::cbegin(path)), home);
  }
  return path;
}

}  // namespace util::file_system
