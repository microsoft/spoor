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
using spoor::instrumentation::config::kForceBinaryOutputKey;
using spoor::instrumentation::config::kFunctionAllowListFileKey;
using spoor::instrumentation::config::kFunctionBlocklistFileKey;
using spoor::instrumentation::config::kInitializeRuntimeKey;
using spoor::instrumentation::config::kInjectInstrumentationKey;
using spoor::instrumentation::config::kMinInstructionThresholdKey;
using spoor::instrumentation::config::kModuleIdKey;
using spoor::instrumentation::config::kOutputFileKey;
using spoor::instrumentation::config::kOutputLanguageKey;
using spoor::instrumentation::config::kOutputSymbolsFileKey;
using spoor::instrumentation::config::OutputLanguage;

TEST(EnvConfig, GetsUserProvidedValue) {  // NOLINT
  const auto get_env = [](const char* key) {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 11>
        environment{
            {kEnableRuntimeKey, "false"},
            {kForceBinaryOutputKey, "true"},
            {kFunctionAllowListFileKey, "/path/to/allow_list.txt"},
            {kFunctionBlocklistFileKey, "/path/to/blocklist.txt"},
            {kInitializeRuntimeKey, "false"},
            {kInjectInstrumentationKey, "false"},
            {kMinInstructionThresholdKey, "42"},
            {kModuleIdKey, "ModuleId"},
            {kOutputFileKey, "/path/to/output_file.ll"},
            {kOutputSymbolsFileKey, "/path/to/file.spoor_symbols"},
            {kOutputLanguageKey, "      iR     "},
        };
    return environment.FirstValueForKey(key).value_or(nullptr).data();
  };
  const Config expected_config{
      .enable_runtime = false,
      .force_binary_output = true,
      .function_allow_list_file = "/path/to/allow_list.txt",
      .function_blocklist_file = "/path/to/blocklist.txt",
      .initialize_runtime = false,
      .inject_instrumentation = false,
      .min_instruction_threshold = 42,
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
                               .force_binary_output = false,
                               .function_allow_list_file = {},
                               .function_blocklist_file = {},
                               .initialize_runtime = true,
                               .inject_instrumentation = true,
                               .min_instruction_threshold = 0,
                               .module_id = {},
                               .output_file = "-",
                               .output_symbols_file = "",
                               .output_language = OutputLanguage::kBitcode};
  ASSERT_EQ(ConfigFromEnv(get_env), expected_config);
}

TEST(EnvConfig, UsesDefaultValueForEmptyStringValues) {  // NOLINT
  const auto get_env = [](const char* key) {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 11>
        environment{{kEnableRuntimeKey, ""},
                    {kForceBinaryOutputKey, ""},
                    {kFunctionAllowListFileKey, ""},
                    {kFunctionBlocklistFileKey, ""},
                    {kInitializeRuntimeKey, ""},
                    {kInjectInstrumentationKey, ""},
                    {kMinInstructionThresholdKey, ""},
                    {kModuleIdKey, ""},
                    {kOutputFileKey, ""},
                    {kOutputSymbolsFileKey, ""},
                    {kOutputLanguageKey, ""}};
    return environment.FirstValueForKey(key).value_or(nullptr).data();
  };
  const Config expected_config{.enable_runtime = true,
                               .force_binary_output = false,
                               .function_allow_list_file = {},
                               .function_blocklist_file = {},
                               .initialize_runtime = true,
                               .inject_instrumentation = true,
                               .min_instruction_threshold = 0,
                               .module_id = {},
                               .output_file = "",
                               .output_symbols_file = "",
                               .output_language = OutputLanguage::kBitcode};
  ASSERT_EQ(ConfigFromEnv(get_env), expected_config);
}

}  // namespace
