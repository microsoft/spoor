#pragma once

#include <cstdlib>
#include <functional>

#include "absl/strings/numbers.h"
#include "util/numeric.h"

namespace util::env {

using GetEnv = std::function<const char*(const char*)>;

template <class T>
auto GetEnvOrDefault(
    const char* key, T default_value,
    const GetEnv& get_env = [](const char* key) { return std::getenv(key); })
    -> T;

template <class T>
requires(std::is_integral_v<T>) auto GetEnvOrDefault(const char* key,
                                                     const T default_value,
                                                     const GetEnv& get_env)
    -> T {
  const auto* user_value = get_env(key);
  if (user_value == nullptr) return default_value;
  uint64 value{};
  const auto success = absl::SimpleAtoi(user_value, &value);
  return success ? value : default_value;
}

}  // namespace util::env
