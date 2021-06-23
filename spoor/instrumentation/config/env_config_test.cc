// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/env_config.h"

#include <string_view>

#include "gtest/gtest.h"
#include "spoor/instrumentation/config/config.h"
#include "util/flat_map/flat_map.h"
#include "util/numeric.h"

namespace {

using spoor::instrumentation::config::Config;
using spoor::instrumentation::config::ConfigFromEnv;
using spoor::instrumentation::config::kEnableRuntimeKey;
using spoor::instrumentation::config::kFiltersFileKey;
using spoor::instrumentation::config::kForceBinaryOutputKey;
using spoor::instrumentation::config::kInitializeRuntimeKey;
using spoor::instrumentation::config::kInjectInstrumentationKey;
using spoor::instrumentation::config::kModuleIdKey;
using spoor::instrumentation::config::kOutputFileKey;
using spoor::instrumentation::config::kOutputLanguageKey;
using spoor::instrumentation::config::kOutputSymbolsFileKey;
using spoor::instrumentation::config::OutputLanguage;

TEST(EnvConfig, GetsUserProvidedValue) {  // NOLINT
  const auto get_env = [](const char* key) {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 9>
        environment{
            {kEnableRuntimeKey, "false"},
            {kFiltersFileKey, "/path/to/filters.toml"},
            {kForceBinaryOutputKey, "true"},
            {kInitializeRuntimeKey, "false"},
            {kInjectInstrumentationKey, "false"},
            {kModuleIdKey, "ModuleId"},
            {kOutputFileKey, "/path/to/output_file.ll"},
            {kOutputSymbolsFileKey, "/path/to/file.spoor_symbols"},
            {kOutputLanguageKey, "      iR     "},
        };
    return environment.FirstValueForKey(key).value_or(nullptr).data();
  };
  const Config expected_config{
      .enable_runtime = false,
      .filters_file = "/path/to/filters.toml",
      .force_binary_output = true,
      .initialize_runtime = false,
      .inject_instrumentation = false,
      .module_id = "ModuleId",
      .output_file = "/path/to/output_file.ll",
      .output_symbols_file = "/path/to/file.spoor_symbols",
      .output_language = OutputLanguage::kIr};
  ASSERT_EQ(ConfigFromEnv(get_env), expected_config);
}

TEST(EnvConfig, UsesDefaultValueWhenNotSpecified) {  // NOLINT
  const auto get_env = [](const char* /*unused*/) -> const char* {
    return nullptr;
  };
  const Config expected_config{.enable_runtime = true,
                               .filters_file = {},
                               .force_binary_output = false,
                               .initialize_runtime = true,
                               .inject_instrumentation = true,
                               .module_id = {},
                               .output_file = "-",
                               .output_symbols_file = "",
                               .output_language = OutputLanguage::kBitcode};
  ASSERT_EQ(ConfigFromEnv(get_env), expected_config);
}

TEST(EnvConfig, UsesDefaultValueForEmptyStringValues) {  // NOLINT
  const auto get_env = [](const char* key) {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 9>
        environment{{kEnableRuntimeKey, ""},
                    {kFiltersFileKey, ""},
                    {kForceBinaryOutputKey, ""},
                    {kInitializeRuntimeKey, ""},
                    {kInjectInstrumentationKey, ""},
                    {kModuleIdKey, ""},
                    {kOutputFileKey, ""},
                    {kOutputSymbolsFileKey, ""},
                    {kOutputLanguageKey, ""}};
    return environment.FirstValueForKey(key).value_or(nullptr).data();
  };
  const Config expected_config{.enable_runtime = true,
                               .filters_file = {},
                               .force_binary_output = false,
                               .initialize_runtime = true,
                               .inject_instrumentation = true,
                               .module_id = {},
                               .output_file = "",
                               .output_symbols_file = "",
                               .output_language = OutputLanguage::kBitcode};
  ASSERT_EQ(ConfigFromEnv(get_env), expected_config);
}

}  // namespace
