// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

#include "absl/strings/ascii.h"
#include "absl/strings/numbers.h"
#include "util/flat_map/flat_map.h"

namespace util::env {

constexpr std::string_view kHomeKey{"HOME"};

using StdGetEnv = std::function<const char*(const char*)>;

template <class T>
auto GetEnv(std::string_view key, const StdGetEnv& get_env)
    -> std::optional<bool>
requires(std::is_same_v<T, bool>);

template <class T>
auto GetEnv(std::string_view key, const StdGetEnv& get_env) -> std::optional<T>
requires(std::is_integral_v<T> && !std::is_same_v<T, bool>);

auto GetEnv(std::string_view key, bool empty_string_is_nullopt,
            const StdGetEnv& get_env) -> std::optional<std::string>;

template <class T, std::size_t Size>
auto GetEnv(std::string_view key,
            const util::flat_map::FlatMap<std::string_view, T, Size>& value_map,
            bool normalize, const StdGetEnv& get_env) -> std::optional<T>;

template <class T>
auto GetEnv(std::string_view key, const StdGetEnv& get_env)
    -> std::optional<bool>
requires(std::is_same_v<T, bool>) {
  const auto* user_value = get_env(key.data());
  if (user_value == nullptr) return {};
  std::string value{user_value};
  absl::StripAsciiWhitespace(&value);
  absl::AsciiStrToLower(&value);
  bool result{};
  const auto success = absl::SimpleAtob(value, &result);
  return success ? std::optional{result} : std::nullopt;
}

template <class T>
auto GetEnv(std::string_view key, const StdGetEnv& get_env) -> std::optional<T>
requires(std::is_integral_v<T> && !std::is_same_v<T, bool>) {
  const auto* user_value = get_env(key.data());
  if (user_value == nullptr) return {};
  T value{};
  const auto success = absl::SimpleAtoi(user_value, &value);
  return success ? std::optional{value} : std::nullopt;
}

template <class T, std::size_t Size>
auto GetEnv(std::string_view key,
            const util::flat_map::FlatMap<std::string_view, T, Size>& value_map,
            const bool normalize, const StdGetEnv& get_env)
    -> std::optional<T> {
  const auto* user_key = get_env(key.data());
  if (user_key == nullptr) return {};
  auto normalized_key = std::string{user_key};
  if (normalize) {
    absl::StripAsciiWhitespace(&normalized_key);
    absl::AsciiStrToLower(&normalized_key);
  }
  return value_map.FirstValueForKey(normalized_key);
}

}  // namespace util::env
