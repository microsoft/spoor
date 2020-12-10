// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <type_traits>

#include "absl/strings/numbers.h"

namespace util::env {

using GetEnv = std::function<const char*(const char*)>;

auto GetEnvOrDefault(const char* key, std::string default_value,
                     const GetEnv& get_env) -> std::string;

auto GetEnvOrDefault(const char* key, std::optional<std::string> default_value,
                     bool empty_string_is_nullopt, const GetEnv& get_env)
    -> std::optional<std::string>;

auto GetEnvOrDefault(const char* key, bool default_value, const GetEnv& get_env)
    -> bool;

template <class T>
auto GetEnvOrDefault(const char* key, T default_value, const GetEnv& get_env)
    -> T requires(std::is_integral_v<T> && !std::is_same_v<T, bool>) {
  const auto* user_value = get_env(key);
  if (user_value == nullptr) return default_value;
  T value{};
  const auto success = absl::SimpleAtoi(user_value, &value);
  return success ? value : default_value;
}

}  // namespace util::env
