// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/file_system/util.h"

#include <iterator>
#include <regex>
#include <sstream>
#include <string>

#include "absl/strings/match.h"
#include "util/env/env.h"

namespace util::file_system {

auto ExpandTilde(std::string path, const env::StdGetEnv& get_env)
    -> std::string {
  using Result = util::result::Result<std::string, util::result::None>;
  if (!(path == "~" || absl::StartsWith(path, "~/"))) return path;
  const auto home_result =
      env::GetEnv(env::kHomeKey, true, get_env).value_or(Result::Ok("~"));
  if (home_result.IsOk()) {
    const auto& home = home_result.Ok();
    path.replace(std::cbegin(path), std::next(std::cbegin(path)), home);
  }
  return path;
}

auto ExpandEnvironmentVariables(const std::string& path,
                                const env::StdGetEnv& get_env) -> std::string {
  const std::regex environment_variable_pattern{"(\\$\\w+)"};
  auto environment_variable_iterator = std::sregex_token_iterator{
      std::cbegin(path), std::cend(path), environment_variable_pattern};
  std::stringstream expanded_path{};
  auto path_iter = std::cbegin(path);
  for (auto match = environment_variable_iterator;
       match != std::sregex_token_iterator{}; ++match) {
    expanded_path << std::string{path_iter, match->first};
    const auto key = std::string{std::next(match->first), match->second};
    const auto fallback_value = std::string{match->first, match->second};
    const auto value = env::GetEnv(key, true, get_env)
                           .value_or(fallback_value)
                           .OkOr(fallback_value);
    expanded_path << value;
    path_iter = match->second;
  }
  expanded_path << std::string{path_iter, std::cend(path)};
  return expanded_path.str();
}

auto ExpandPath(std::string path, const PathExpansionOptions& options)
    -> std::string {
  if (options.expand_tilde) path = ExpandTilde(path, options.get_env);
  if (options.expand_environment_variables) {
    path = ExpandEnvironmentVariables(path, options.get_env);
  }
  return path;
}

}  // namespace util::file_system
