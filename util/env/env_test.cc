// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/env/env.h"

#include <optional>
#include <string>
#include <string_view>

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "gtest/gtest.h"
#include "util/numeric.h"

namespace {

using util::env::GetEnv;

TEST(GetEnv, Int) {  // NOLINT
  for (const auto value : {0, 1, 7, 42, 1'000'000}) {
    const auto env_value = std::to_string(value);
    const auto retrieved_env_value = GetEnv<int>(
        "KEY",
        [&env_value](const char* /*unused*/) { return env_value.data(); });
    ASSERT_TRUE(retrieved_env_value.has_value());
    ASSERT_TRUE(retrieved_env_value.value().IsOk());
    ASSERT_EQ(retrieved_env_value.value().Ok(), value);
  }
  for (const auto* env_value : {"", " ", "bad value"}) {
    const auto retrieved_env_value = GetEnv<int>(
        "KEY", [&env_value](const char* /*unused*/) { return env_value; });
    ASSERT_TRUE(retrieved_env_value.has_value());
    ASSERT_TRUE(retrieved_env_value.value().IsErr());
  }
  const auto retrieved_env_value =
      GetEnv<int>("KEY", [](const char* /*unused*/) { return nullptr; });
  ASSERT_FALSE(retrieved_env_value.has_value());
}

TEST(GetEnv, String) {  // NOLINT
  for (const auto* env_value : {"", "bar", "baz", "/path/to/file.extension"}) {
    constexpr auto empty_string_is_nullopt{false};
    const auto retrieved_env_value =
        GetEnv("KEY", empty_string_is_nullopt,
               [&env_value](const char* /*unused*/) { return env_value; });
    ASSERT_TRUE(retrieved_env_value.has_value());
    ASSERT_TRUE(retrieved_env_value.value().IsOk());
    ASSERT_EQ(retrieved_env_value.value().Ok(), env_value);
  }
  {
    constexpr auto empty_string_is_nullopt{true};
    const auto retrieved_env_value =
        GetEnv("KEY", empty_string_is_nullopt,
               [](const char* /*unused*/) { return ""; });
    ASSERT_FALSE(retrieved_env_value.has_value());
  }
  {
    constexpr auto empty_string_is_nullopt{false};
    const auto retrieved_env_value =
        GetEnv("KEY", empty_string_is_nullopt,
               [](const char* /*unused*/) { return nullptr; });
    ASSERT_FALSE(retrieved_env_value.has_value());
  }
}

TEST(GetEnv, Bool) {  // NOLINT
  for (const auto* env_value : {"0", "no", "false"}) {
    const auto retrieved_env_value = GetEnv<bool>(
        "KEY", [&env_value](const char* /*unused*/) { return env_value; });
    ASSERT_TRUE(retrieved_env_value.has_value());
    ASSERT_TRUE(retrieved_env_value.value().IsOk());
    ASSERT_FALSE(retrieved_env_value.value().Ok());
  }
  for (const auto* env_value : {"1", "yes", "true"}) {
    const auto retrieved_env_value = GetEnv<bool>(
        "KEY", [&env_value](const char* /*unused*/) { return env_value; });
    ASSERT_TRUE(retrieved_env_value.has_value());
    ASSERT_TRUE(retrieved_env_value.value().IsOk());
    ASSERT_TRUE(retrieved_env_value.value().Ok());
  }
  const auto retrieved_value =
      GetEnv<bool>("KEY", [](const char* /*unused*/) { return nullptr; });
  ASSERT_FALSE(retrieved_value.has_value());
  for (const auto* bad_env_value : {"", "bad value"}) {
    const auto retrieved_value = GetEnv<bool>(
        "KEY",
        [&bad_env_value](const char* /*unused*/) { return bad_env_value; });
    ASSERT_TRUE(retrieved_value.has_value());
    ASSERT_TRUE(retrieved_value.value().IsErr());
  }
}

TEST(GetEnv, ValueMap) {  // NOLINT
  constexpr util::flat_map::FlatMap<std::string_view, uint32, 3> value_map{
      {"zero", 0}, {"one", 1}, {"two", 2}};
  for (const auto& [raw_key, raw_value] : value_map) {
    const std::string key{raw_key};
    const auto messy_key =
        absl::StrCat("   ", absl::AsciiStrToUpper(key), "    ");
    const auto retrieved_env_value =
        GetEnv("KEY", value_map, false,
               [key](const char* /*unused*/) { return key.data(); });
    ASSERT_TRUE(retrieved_env_value.has_value());
    ASSERT_TRUE(retrieved_env_value.value().IsOk());
    ASSERT_EQ(retrieved_env_value.value().Ok(), raw_value);
    const auto retrieved_env_value_not_normalized = GetEnv(
        "KEY", value_map, false,
        [messy_key](const char* /*unused*/) { return messy_key.data(); });
    ASSERT_TRUE(retrieved_env_value_not_normalized.has_value());
    ASSERT_TRUE(retrieved_env_value_not_normalized.value().IsErr());
    const auto retrieved_env_value_normalized = GetEnv(
        "KEY", value_map, true,
        [messy_key](const char* /*unused*/) { return messy_key.data(); });
    ASSERT_TRUE(retrieved_env_value_normalized.has_value());
    ASSERT_TRUE(retrieved_env_value_normalized.value().IsOk());
    ASSERT_EQ(retrieved_env_value_normalized.value().Ok(), raw_value);
  }
  const auto retrieved_env_value_nullptr = GetEnv(
      "KEY", value_map, true, [](const char* /*unused*/) { return nullptr; });
  ASSERT_FALSE(retrieved_env_value_nullptr.has_value());
  const auto retrieved_env_value_invalid = GetEnv(
      "KEY", value_map, true, [](const char* /*unused*/) { return "invalid"; });
  ASSERT_TRUE(retrieved_env_value_invalid.has_value());
  ASSERT_TRUE(retrieved_env_value_invalid.value().IsErr());
}

}  // namespace
