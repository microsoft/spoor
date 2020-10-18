#pragma once

#include <cstdlib>
#include <functional>

#include "absl/strings/numbers.h"

namespace util::env {

using GetEnv = std::function<const char*(const char*)>;

auto GetEnvOrDefault(const char* key, std::string default_value,
                     const std::function<const char*(const char*)>& get_env)
    -> std::string;

auto GetEnvOrDefault(const char* key, bool default_value,
                     const std::function<const char*(const char*)>& get_env)
    -> bool;

template <class T, class = std::enable_if_t<std::is_integral_v<T> &&
                                            !std::is_same_v<T, bool>>>
auto GetEnvOrDefault(const char* key, const T default_value,
                     const GetEnv& get_env) -> T {
  const auto* user_value = get_env(key);
  if (user_value == nullptr) return default_value;
  T value{};
  const auto success = absl::SimpleAtoi(user_value, &value);
  return success ? value : default_value;
}

}  // namespace util::env
