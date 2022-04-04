// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/env_source.h"

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "util/file_system/file_system_mock.h"
#include "util/flat_map/flat_map.h"

namespace {

using spoor::instrumentation::config::EnvSource;
using spoor::instrumentation::config::kEnableRuntimeEnvKey;
using spoor::instrumentation::config::kEnvConfigKeys;
using spoor::instrumentation::config::kFiltersFileEnvKey;
using spoor::instrumentation::config::kForceBinaryOutputEnvKey;
using spoor::instrumentation::config::kInitializeRuntimeEnvKey;
using spoor::instrumentation::config::kInjectInstrumentationEnvKey;
using spoor::instrumentation::config::kModuleIdEnvKey;
using spoor::instrumentation::config::kOutputFileEnvKey;
using spoor::instrumentation::config::kOutputLanguageEnvKey;
using spoor::instrumentation::config::kOutputSymbolsFileEnvKey;
using spoor::instrumentation::config::OutputLanguage;
using spoor::instrumentation::config::Source;
using testing::Return;
using util::env::kHomeKey;
using util::file_system::testing::FileSystemMock;

TEST(EnvSource, InitialState) {  // NOLINT
  const auto get_env = [](const char* /*unused*/) { return nullptr; };
  const EnvSource env_source{{
      .path_expansion_options{
          .get_env = get_env,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_system = std::make_unique<FileSystemMock>(),
      .get_env = get_env,
  }};
  ASSERT_FALSE(env_source.IsRead());

  ASSERT_FALSE(env_source.GetEnableRuntime().has_value());
  ASSERT_FALSE(env_source.GetFiltersFile().has_value());
  ASSERT_FALSE(env_source.GetForceBinaryOutput().has_value());
  ASSERT_FALSE(env_source.GetInitializeRuntime().has_value());
  ASSERT_FALSE(env_source.GetInjectInstrumentation().has_value());
  ASSERT_FALSE(env_source.GetModuleId().has_value());
  ASSERT_FALSE(env_source.GetOutputFile().has_value());
  ASSERT_FALSE(env_source.GetOutputLanguage().has_value());
  ASSERT_FALSE(env_source.GetOutputSymbolsFile().has_value());
}

TEST(EnvSource, ConfigsAreNulloptIfNotSet) {  // NOLINT
  const auto get_env = [](const char* /*unused*/) { return nullptr; };
  const std::filesystem::path home{"/usr/you"};
  auto file_system = std::make_unique<FileSystemMock>();
  EXPECT_CALL(*file_system, CurrentPath()).WillOnce(Return(home));
  EnvSource env_source{{
      .path_expansion_options{
          .get_env = get_env,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_system = std::move(file_system),
      .get_env = get_env,
  }};
  const auto errors = env_source.Read();
  ASSERT_TRUE(env_source.IsRead());
  ASSERT_TRUE(errors.empty());

  ASSERT_FALSE(env_source.GetEnableRuntime().has_value());
  ASSERT_FALSE(env_source.GetFiltersFile().has_value());
  ASSERT_FALSE(env_source.GetForceBinaryOutput().has_value());
  ASSERT_FALSE(env_source.GetInitializeRuntime().has_value());
  ASSERT_FALSE(env_source.GetInjectInstrumentation().has_value());
  ASSERT_FALSE(env_source.GetModuleId().has_value());
  ASSERT_FALSE(env_source.GetOutputFile().has_value());
  ASSERT_FALSE(env_source.GetOutputLanguage().has_value());
  ASSERT_FALSE(env_source.GetOutputSymbolsFile().has_value());
}

TEST(EnvSource, ParsesConfigs) {  // NOLINT
  const auto get_env = [](const char* key) {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view,
                                      kEnvConfigKeys.size()>
        environment{
            {kEnableRuntimeEnvKey, "false"},
            {kFiltersFileEnvKey, "/path/to/filters.toml"},
            {kForceBinaryOutputEnvKey, "true"},
            {kInitializeRuntimeEnvKey, "false"},
            {kInjectInstrumentationEnvKey, "false"},
            {kModuleIdEnvKey, "id"},
            {kOutputFileEnvKey, "/path/to/output.ll"},
            {kOutputLanguageEnvKey, "ir"},
            {kOutputSymbolsFileEnvKey, "/path/to/symbols.spoor_symbols"},
        };
    return environment.FirstValueForKey(key).value().data();
  };
  const std::filesystem::path home{"/usr/you"};
  auto file_system = std::make_unique<FileSystemMock>();
  EXPECT_CALL(*file_system, CurrentPath()).WillOnce(Return(home));
  EnvSource env_source{{
      .path_expansion_options{
          .get_env = get_env,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_system = std::move(file_system),
      .get_env = get_env,
  }};

  const auto errors = env_source.Read();
  ASSERT_TRUE(env_source.IsRead());
  ASSERT_TRUE(errors.empty());

  ASSERT_TRUE(env_source.GetEnableRuntime().has_value());
  ASSERT_FALSE(env_source.GetEnableRuntime().value());

  ASSERT_TRUE(env_source.GetFiltersFile().has_value());
  ASSERT_EQ(env_source.GetFiltersFile().value(), "/path/to/filters.toml");

  ASSERT_TRUE(env_source.GetForceBinaryOutput().has_value());
  ASSERT_TRUE(env_source.GetForceBinaryOutput().value());

  ASSERT_TRUE(env_source.GetInitializeRuntime().has_value());
  ASSERT_FALSE(env_source.GetInitializeRuntime().value());

  ASSERT_TRUE(env_source.GetInjectInstrumentation().has_value());
  ASSERT_FALSE(env_source.GetInjectInstrumentation().value());

  ASSERT_TRUE(env_source.GetModuleId().has_value());
  ASSERT_EQ(env_source.GetModuleId().value(), "id");

  ASSERT_TRUE(env_source.GetOutputFile().has_value());
  ASSERT_EQ(env_source.GetOutputFile().value(), "/path/to/output.ll");

  ASSERT_TRUE(env_source.GetOutputLanguage().has_value());
  ASSERT_EQ(env_source.GetOutputLanguage().value(), OutputLanguage::kIr);

  ASSERT_TRUE(env_source.GetOutputSymbolsFile().has_value());
  ASSERT_EQ(env_source.GetOutputSymbolsFile().value(),
            "/path/to/symbols.spoor_symbols");
}

TEST(EnvSource, ExpandsPath) {  // NOLINT
  constexpr std::string_view home{"/usr/you"};
  const auto get_env = [&](const char* key) -> const char* {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 5>
        environment{
            {kHomeKey, home},
            {"FOO", "bar"},
            {kFiltersFileEnvKey, "~/$FOO/filters.toml"},
            {kOutputFileEnvKey, "~/$FOO/output.ll"},
            {kOutputSymbolsFileEnvKey, "~/$FOO/symbols.spoor_symbols"},
        };
    auto value = environment.FirstValueForKey(key);
    if (value.has_value()) return value.value().data();
    return nullptr;
  };
  auto file_system = std::make_unique<FileSystemMock>();
  EXPECT_CALL(*file_system, CurrentPath())
      .WillOnce(Return(std::filesystem::path{home}));
  EnvSource env_source{{
      .path_expansion_options{
          .get_env = get_env,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_system = std::move(file_system),
      .get_env = get_env,
  }};
  const auto errors = env_source.Read();
  ASSERT_TRUE(env_source.IsRead());
  ASSERT_TRUE(errors.empty());

  ASSERT_FALSE(env_source.GetEnableRuntime().has_value());

  ASSERT_TRUE(env_source.GetFiltersFile().has_value());
  ASSERT_EQ(env_source.GetFiltersFile().value(), "/usr/you/bar/filters.toml");

  ASSERT_FALSE(env_source.GetForceBinaryOutput().has_value());

  ASSERT_FALSE(env_source.GetInitializeRuntime().has_value());

  ASSERT_FALSE(env_source.GetInjectInstrumentation().has_value());

  ASSERT_FALSE(env_source.GetModuleId().has_value());

  ASSERT_TRUE(env_source.GetOutputFile().has_value());
  ASSERT_EQ(env_source.GetOutputFile().value(), "/usr/you/bar/output.ll");

  ASSERT_FALSE(env_source.GetOutputLanguage().has_value());

  ASSERT_TRUE(env_source.GetOutputSymbolsFile().has_value());
  ASSERT_EQ(env_source.GetOutputSymbolsFile().value(),
            "/usr/you/bar/symbols.spoor_symbols");
}

TEST(EnvSource, ConvertsRelativeToAbsolutePath) {  // NOLINT
  const auto get_env = [](const char* key) -> const char* {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 3>
        environment{
            {kFiltersFileEnvKey, "path/to/filters.toml"},
            {kOutputFileEnvKey, "path/to/output.ll"},
            {kOutputSymbolsFileEnvKey, "path/to/symbols.spoor_symbols"},
        };
    auto value = environment.FirstValueForKey(key);
    if (value.has_value()) return value.value().data();
    return nullptr;
  };
  const std::filesystem::path home{"/usr/you"};
  auto file_system = std::make_unique<FileSystemMock>();
  EXPECT_CALL(*file_system, CurrentPath()).WillOnce(Return(home));
  EnvSource env_source{{
      .path_expansion_options{
          .get_env = get_env,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_system = std::move(file_system),
      .get_env = get_env,
  }};
  const auto errors = env_source.Read();
  ASSERT_TRUE(env_source.IsRead());
  ASSERT_TRUE(errors.empty());

  ASSERT_FALSE(env_source.GetEnableRuntime().has_value());

  ASSERT_TRUE(env_source.GetFiltersFile().has_value());
  ASSERT_EQ(env_source.GetFiltersFile().value(),
            "/usr/you/path/to/filters.toml");

  ASSERT_FALSE(env_source.GetForceBinaryOutput().has_value());

  ASSERT_FALSE(env_source.GetInitializeRuntime().has_value());

  ASSERT_FALSE(env_source.GetInjectInstrumentation().has_value());

  ASSERT_FALSE(env_source.GetModuleId().has_value());

  ASSERT_TRUE(env_source.GetOutputFile().has_value());
  ASSERT_EQ(env_source.GetOutputFile().value(), "/usr/you/path/to/output.ll");

  ASSERT_FALSE(env_source.GetOutputLanguage().has_value());

  ASSERT_TRUE(env_source.GetOutputSymbolsFile().has_value());
  ASSERT_EQ(env_source.GetOutputSymbolsFile().value(),
            "/usr/you/path/to/symbols.spoor_symbols");
}

TEST(EnvSource, NormalizesOutputLanguage) {  // NOLINT
  const auto get_env = [](const char* key) -> const char* {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 1>
        environment{{kOutputLanguageEnvKey, "   bItCoDe   "}};
    auto value = environment.FirstValueForKey(key);
    if (value.has_value()) return value.value().data();
    return nullptr;
  };
  const std::filesystem::path home{"/usr/you"};
  auto file_system = std::make_unique<FileSystemMock>();
  EXPECT_CALL(*file_system, CurrentPath()).WillOnce(Return(home));
  EnvSource env_source{{
      .path_expansion_options{
          .get_env = get_env,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_system = std::move(file_system),
      .get_env = get_env,
  }};
  const auto errors = env_source.Read();
  ASSERT_TRUE(env_source.IsRead());
  ASSERT_TRUE(errors.empty());

  ASSERT_FALSE(env_source.GetEnableRuntime().has_value());

  ASSERT_FALSE(env_source.GetFiltersFile().has_value());

  ASSERT_FALSE(env_source.GetForceBinaryOutput().has_value());

  ASSERT_FALSE(env_source.GetInitializeRuntime().has_value());

  ASSERT_FALSE(env_source.GetInjectInstrumentation().has_value());

  ASSERT_FALSE(env_source.GetModuleId().has_value());

  ASSERT_FALSE(env_source.GetOutputFile().has_value());

  ASSERT_TRUE(env_source.GetOutputLanguage().has_value());
  ASSERT_EQ(env_source.GetOutputLanguage().value(), OutputLanguage::kBitcode);

  ASSERT_FALSE(env_source.GetOutputSymbolsFile().has_value());
}

TEST(EnvSource, ReadsConfigOnce) {  // NOLINT
  const auto get_env = [](const char* /*unused*/) { return nullptr; };
  const std::filesystem::path home{"/usr/you"};
  auto file_system = std::make_unique<FileSystemMock>();
  EXPECT_CALL(*file_system, CurrentPath()).WillOnce(Return(home));
  EnvSource env_source{{
      .path_expansion_options{
          .get_env = get_env,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_system = std::move(file_system),
      .get_env = get_env,
  }};
  ASSERT_FALSE(env_source.IsRead());
  for (auto run{0}; run < 3; ++run) {
    const auto errors = env_source.Read();
    ASSERT_TRUE(env_source.IsRead());
    ASSERT_TRUE(errors.empty());
  }
}

TEST(EnvSource, HandlesBadValue) {  // NOLINT
  constexpr auto bad_values{5};
  const auto get_env = [](const char* key) -> const char* {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view,
                                      bad_values>
        environment{
            {kEnableRuntimeEnvKey, "bad value"},
            {kForceBinaryOutputEnvKey, "bad value"},
            {kInitializeRuntimeEnvKey, "bad value"},
            {kInjectInstrumentationEnvKey, "bad value"},
            {kOutputLanguageEnvKey, "bad value"},
        };
    auto value = environment.FirstValueForKey(key);
    if (value.has_value()) return value.value().data();
    return nullptr;
  };
  const std::filesystem::path home{"/usr/you"};
  auto file_system = std::make_unique<FileSystemMock>();
  EXPECT_CALL(*file_system, CurrentPath()).WillOnce(Return(home));
  EnvSource env_source{{
      .path_expansion_options{
          .get_env = get_env,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_system = std::move(file_system),
      .get_env = get_env,
  }};
  const auto errors = env_source.Read();
  ASSERT_TRUE(env_source.IsRead());

  ASSERT_EQ(errors.size(), bad_values);

  const auto error_types = [&] {
    std::vector<Source::ReadError::Type> error_types{};
    std::transform(std::cbegin(errors), std::cend(errors),
                   std::back_inserter(error_types),
                   [](const auto& error) { return error.type; });
    return error_types;
  }();
  const auto expected_error_types =
      std::vector(bad_values, Source::ReadError::Type::kUnknownValue);
  ASSERT_EQ(error_types, expected_error_types);
}

}  // namespace
