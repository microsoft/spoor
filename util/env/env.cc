// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/env/env.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>

#include "absl/strings/numbers.h"

namespace util::env {

auto GetEnvOrDefault(const char* key, std::string default_value,
                     const GetEnv& get_env) -> std::string {
  const auto* user_value = get_env(key);
  if (user_value == nullptr) return default_value;
  return std::string{user_value};
}

auto GetEnvOrDefault(const char* key, std::optional<std::string> default_value,
                     const bool empty_string_is_nullopt, const GetEnv& get_env)
    -> std::optional<std::string> {
  const auto* user_value = get_env(key);
  if (user_value == nullptr) return default_value;
  auto user_value_string = std::string{user_value};
  if (user_value_string.empty() && empty_string_is_nullopt) return {};
  return std::move(user_value_string);
}

auto GetEnvOrDefault(const char* key, const bool default_value,
                     const GetEnv& get_env) -> bool {
  const auto* user_value = get_env(key);
  if (user_value == nullptr) return default_value;
  std::string value{user_value};
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  bool result{};
  const auto success = absl::SimpleAtob(value, &result);
  return success ? result : default_value;
}

}  // namespace util::env
