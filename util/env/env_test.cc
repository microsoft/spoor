// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/env/env.h"

#include <string>
#include <unordered_map>

#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "gtest/gtest.h"
#include "util/numeric.h"

namespace {

using util::env::GetEnvOrDefault;

TEST(GetEnvOrDefault, Int64) {  // NOLINT
  constexpr int64 default_value{42};
  for (const auto value : {0, 1, 7, 42, 1'000'000}) {
    auto env_value = std::to_string(value);
    const auto retrieved_env_value = GetEnvOrDefault(
        "KEY", default_value,
        [&env_value](const char* /*unused*/) { return env_value.data(); });
    ASSERT_EQ(retrieved_env_value, value);
  }
  for (const auto* env_value : {"", " ", "bad value"}) {
    const auto retrieved_env_value = GetEnvOrDefault(
        "KEY", default_value,
        [&env_value](const char* /*unused*/) { return env_value; });
    ASSERT_EQ(retrieved_env_value, default_value);
  }
  const auto retrieved_default_value = GetEnvOrDefault(
      "KEY", default_value, [](const char* /*unused*/) { return nullptr; });
  ASSERT_EQ(retrieved_default_value, default_value);
}

TEST(GetEnvOrDefault, String) {  // NOLINT
  const std::string default_value{"foo"};
  for (const auto* env_value : {"", "bar", "baz", "/path/to/file.extension"}) {
    const auto retrieved_env_value = GetEnvOrDefault(
        "KEY", default_value,
        [&env_value](const char* /*unused*/) { return env_value; });
    ASSERT_EQ(retrieved_env_value, env_value);
  }
  const auto retrieved_default_value = GetEnvOrDefault(
      "KEY", default_value, [](const char* /*unused*/) { return nullptr; });
  ASSERT_EQ(retrieved_default_value, default_value);
}

TEST(GetEnvOrDefault, OptionalString) {  // NOLINT
  const std::string default_value{"foo"};
  for (const auto* env_value : {"bar", "baz", "/path/to/file.extension"}) {
    const auto retrieved_env_value = GetEnvOrDefault(
        "KEY", default_value, true,
        [&env_value](const char* /*unused*/) { return env_value; });
    ASSERT_EQ(retrieved_env_value, std::optional{env_value});
  }
  for (const auto empty_string_is_nullopt : {false, true}) {
    const auto retrieved_env_value =
        GetEnvOrDefault("KEY", default_value, empty_string_is_nullopt,
                        [](const char* /*unused*/) { return ""; });
    if (empty_string_is_nullopt) {
      ASSERT_EQ(retrieved_env_value, std::nullopt);
    } else {
      ASSERT_EQ(retrieved_env_value, std::optional{""});
    }
  }
  const auto retrieved_default_value = GetEnvOrDefault(
      "KEY", default_value, [](const char* /*unused*/) { return nullptr; });
  ASSERT_EQ(retrieved_default_value, default_value);
}

TEST(GetEnvOrDefault, Bool) {  // NOLINT
  for (const auto* env_value : {"0", "no", "false"}) {
    constexpr bool value{false};
    const auto retrieved_env_value = GetEnvOrDefault(
        "KEY", !value,
        [&env_value](const char* /*unused*/) { return env_value; });
    ASSERT_EQ(retrieved_env_value, value);
  }

  for (const auto* env_value : {"1", "yes", "true"}) {
    constexpr bool value{true};
    const auto retrieved_env_value = GetEnvOrDefault(
        "KEY", !value,
        [&env_value](const char* /*unused*/) { return env_value; });
    ASSERT_EQ(retrieved_env_value, value);
  }

  for (const auto default_value : {false, true}) {
    const auto retrieved_value = GetEnvOrDefault(
        "KEY", default_value, [](const char* /*unused*/) { return nullptr; });
    ASSERT_EQ(retrieved_value, default_value);
    for (const auto* bad_env_value : {"", "bad value"}) {
      const auto retrieved_value = GetEnvOrDefault(
          "KEY", default_value,
          [&bad_env_value](const char* /*unused*/) { return bad_env_value; });
      ASSERT_EQ(retrieved_value, default_value);
    }
  }
}

TEST(GetEnvOrDefault, ValueMap) {  // NOLINT
  constexpr uint32 default_value{3};
  const std::unordered_map<std::string_view, uint32> value_map{
      {"zero", 0},
      {"one", 1},
      {"two", 2},
  };
  for (const auto& [key_view, value] : value_map) {
    const std::string key{key_view};
    const auto messy_key =
        absl::StrCat("   ", absl::AsciiStrToUpper(key), "    ");
    const auto retrieved_env_value =
        GetEnvOrDefault("KEY", default_value, value_map, false,
                        [key](const char* /*unused*/) { return key.data(); });
    ASSERT_EQ(retrieved_env_value, value);
    const auto retrieved_env_value_not_normalized = GetEnvOrDefault(
        "KEY", default_value, value_map, false,
        [messy_key](const char* /*unused*/) { return messy_key.data(); });
    ASSERT_EQ(retrieved_env_value_not_normalized, default_value);
    const auto retrieved_env_value_normalized = GetEnvOrDefault(
        "KEY", default_value, value_map, true,
        [messy_key](const char* /*unused*/) { return messy_key.data(); });
    ASSERT_EQ(retrieved_env_value_normalized, value);
  }
  const auto retrieved_env_value_nullptr =
      GetEnvOrDefault("KEY", default_value, value_map, true,
                      [](const char* /*unused*/) { return nullptr; });
  ASSERT_EQ(retrieved_env_value_nullptr, default_value);
  const auto retrieved_env_value_invalid =
      GetEnvOrDefault("KEY", default_value, value_map, true,
                      [](const char* /*unused*/) { return "invalid"; });
  ASSERT_EQ(retrieved_env_value_invalid, default_value);
}

}  // namespace
