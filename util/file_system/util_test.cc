// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/file_system/util.h"

#include <array>
#include <cstring>
#include <string>
#include <string_view>

#include "gtest/gtest.h"
#include "util/env/env.h"

namespace {

using util::file_system::ExpandPath;
using util::file_system::PathExpansionOptions;

constexpr std::string_view kHome{"/usr/you"};

TEST(ExpandPath, ExpandsTilde) {  // NOLINT
  const auto get_env = [](const char* key) {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 1>
        environment{{util::env::kHomeKey, kHome}};
    return environment.FirstValueForKey(key).value_or(nullptr).data();
  };
  const PathExpansionOptions options{
      .get_env = get_env,
      .expand_tilde = true,
      .expand_environment_variables = false,
  };

  ASSERT_EQ(ExpandPath("~", options), "/usr/you");

  constexpr auto path{"~/path/to/file.extension"};
  const auto expanded_path = ExpandPath(path, options);
  ASSERT_EQ(expanded_path, "/usr/you/path/to/file.extension");
}

TEST(ExpandPath, DoesNotExpandInvalidTilde) {  // NOLINT
  const std::vector<std::string> paths{{
      "/usr/you/path/to/file.extension",
      "~path/to/file.extension",
      "/usr/you/~/to/file.extension",
  }};
  const auto get_env = [](const char* key) {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 1>
        environment{{util::env::kHomeKey, kHome}};
    return environment.FirstValueForKey(key).value_or(nullptr).data();
  };
  const PathExpansionOptions options{
      .get_env = get_env,
      .expand_tilde = true,
      .expand_environment_variables = false,
  };
  for (const auto& path : paths) {
    const auto expanded_path = ExpandPath(path, options);
    ASSERT_EQ(expanded_path, path);
  }
}

// NOLINTNEXTLINE
TEST(ExpandPath, HandlesNonexistentHomeEnvironmentVariableExpandingTilde) {
  constexpr auto path{"~/path/to/file.extension"};
  const auto get_env = [](const char* /*unused*/) { return nullptr; };
  const PathExpansionOptions options{
      .get_env = get_env,
      .expand_tilde = true,
      .expand_environment_variables = false,
  };
  const auto expanded_path = ExpandPath(path, options);
  ASSERT_EQ(expanded_path, path);
}

TEST(ExpandPath, ExpandsEnvironmentVariables) {  // NOLINT
  constexpr auto path{"$HOME/path/$FOO_BAR/file.extension"};
  const auto get_env = [](const char* key) {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 2>
        environment{
            {"HOME", kHome},
            {"FOO_BAR", "baz"},
        };
    return environment.FirstValueForKey(key).value_or(nullptr).data();
  };
  const PathExpansionOptions options{
      .get_env = get_env,
      .expand_tilde = false,
      .expand_environment_variables = true,
  };
  const auto expanded_path = ExpandPath(path, options);
  ASSERT_EQ(expanded_path, "/usr/you/path/baz/file.extension");
}

TEST(ExpandPath, ExpandsTildeAndEnvironmentVariables) {  // NOLINT
  constexpr auto path{"~/path/$FOO_BAR/file.extension"};
  const auto get_env = [](const char* key) {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 2>
        environment{
            {"HOME", kHome},
            {"FOO_BAR", "baz"},
        };
    return environment.FirstValueForKey(key).value_or(nullptr).data();
  };
  const PathExpansionOptions options{
      .get_env = get_env,
      .expand_tilde = true,
      .expand_environment_variables = true,
  };
  const auto expanded_path = ExpandPath(path, options);
  ASSERT_EQ(expanded_path, "/usr/you/path/baz/file.extension");
}

// NOLINTNEXTLINE
TEST(ExpandPath, HandlesNonexistentEnvironmentVariablesExpandingEnvironment) {
  constexpr auto path{"/path/to/$DOES_NOT_EXIST/file.extension"};
  const auto get_env = [](const char* /*unused*/) { return nullptr; };
  const PathExpansionOptions options{
      .get_env = get_env,
      .expand_tilde = false,
      .expand_environment_variables = true,
  };
  const auto expanded_path = ExpandPath(path, options);
  ASSERT_EQ(expanded_path, path);
}

}  // namespace
