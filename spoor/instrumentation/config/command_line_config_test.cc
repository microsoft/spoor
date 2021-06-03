// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "command_line_config.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string_view>
#include <vector>

#include "external/com_microsoft_gsl/_virtual_includes/gsl/gsl/util"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/instrumentation/config/config.h"
#include "spoor/instrumentation/config/env_config.h"
#include "util/flat_map/flat_map.h"

namespace {

using spoor::instrumentation::config::Config;
using spoor::instrumentation::config::ConfigFromCommandLineOrEnv;
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
using spoor::instrumentation::config::kOutputLanguages;
using spoor::instrumentation::config::kOutputSymbolsFileKey;
using spoor::instrumentation::config::OutputLanguage;

auto MakeArgv(const std::vector<std::string_view>& args) -> std::vector<char*> {
  std::vector<char*> argv{};
  argv.reserve(args.size());
  std::transform(
      std::begin(args), std::end(args), std::back_inserter(argv),
      // `absl::ParseCommandLine` requires a (non-const) char** as an argument.
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
      [](auto string_view) { return const_cast<char*>(string_view.data()); });
  return argv;
}

TEST(CommandLineConfig, ParsesCommandLine) {  // NOLINT
  const auto get_env = [](const char* /*unused*/) -> const char* {
    return nullptr;
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
  auto argv =
      MakeArgv({"spoor_opt", "--enable_runtime=false",
                "--function_allow_list_file=/path/to/allow_list.txt",
                "--function_blocklist_file=/path/to/blocklist.txt",
                "--initialize_runtime=false", "--inject_instrumentation=false",
                "--min_instruction_threshold=42", "--module_id=ModuleId",
                "--output_file=/path/to/output_file.ll",
                "--output_symbols_file=/path/to/file.spoor_symbols",
                "--output_language=ir"});
  const auto expected_positional_args = MakeArgv({argv.front()});
  const auto [config, positional_args] = ConfigFromCommandLineOrEnv(
      gsl::narrow_cast<int>(argv.size()), argv.data(), get_env);
  ASSERT_EQ(config, expected_config);
  ASSERT_EQ(positional_args, expected_positional_args);
}

TEST(CommandLineConfig, UsesDefaultValueWhenNotSpecified) {  // NOLINT
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
  auto argv = MakeArgv({"spoor_opt"});
  const auto [config, positional_args] = ConfigFromCommandLineOrEnv(
      gsl::narrow_cast<int>(argv.size()), argv.data(), get_env);
  ASSERT_EQ(config, expected_config);
  ASSERT_EQ(positional_args, argv);
}

TEST(CommandLineConfig, UsesEnvironmentValueWhenNotSpecified) {  // NOLINT
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
            {kOutputLanguageKey, "ir"},
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
  auto argv = MakeArgv({"spoor_opt"});
  const auto [config, positional_args] = ConfigFromCommandLineOrEnv(
      gsl::narrow_cast<int>(argv.size()), argv.data(), get_env);
  ASSERT_EQ(config, expected_config);
  ASSERT_EQ(positional_args, argv);
}

TEST(CommandLineConfig, OverridesEnvironment) {  // NOLINT
  const auto get_env = [](const char* key) -> const char* {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 11>
        environment{
            {kEnableRuntimeKey, "true"},
            {kForceBinaryOutputKey, "false"},
            {kFunctionAllowListFileKey, "/path/to/other/allow_list.txt"},
            {kFunctionBlocklistFileKey, "/path/to/other/blocklist.txt"},
            {kInitializeRuntimeKey, "true"},
            {kInjectInstrumentationKey, "true"},
            {kMinInstructionThresholdKey, "43"},
            {kModuleIdKey, "OtherModuleId"},
            {kOutputFileKey, "/path/to/other/output_file.ll"},
            {kOutputSymbolsFileKey, "/path/to/other/file.spoor_symbols"},
            {kOutputLanguageKey, "bitcode"}};
    return environment.FirstValueForKey(key).value_or(nullptr).data();
  };
  const Config expected_config{
      .enable_runtime = false,
      .force_binary_output = false,
      .function_allow_list_file = "/path/to/allow_list.txt",
      .function_blocklist_file = "/path/to/blocklist.txt",
      .initialize_runtime = false,
      .inject_instrumentation = false,
      .min_instruction_threshold = 42,
      .module_id = "ModuleId",
      .output_file = "/path/to/output_file.ll",
      .output_symbols_file = "/path/to/file.spoor_symbols",
      .output_language = OutputLanguage::kIr};
  auto argv =
      MakeArgv({"spoor_opt", "--enable_runtime=false",
                "--function_allow_list_file=/path/to/allow_list.txt",
                "--function_blocklist_file=/path/to/blocklist.txt",
                "--initialize_runtime=false", "--inject_instrumentation=false",
                "--min_instruction_threshold=42", "--module_id=ModuleId",
                "--output_file=/path/to/output_file.ll",
                "--output_symbols_file=/path/to/file.spoor_symbols",
                "--output_language=ir"});
  const auto expected_positional_args = MakeArgv({argv.front()});
  const auto [config, positional_args] = ConfigFromCommandLineOrEnv(
      gsl::narrow_cast<int>(argv.size()), argv.data(), get_env);
  ASSERT_EQ(config, expected_config);
  ASSERT_EQ(positional_args, expected_positional_args);
}

TEST(CommandLineConfig, PositionalArguments) {  // NOLINT
  const auto get_env = [](const char* /*unused*/) -> const char* {
    return nullptr;
  };
  auto argv = MakeArgv({"spoor_opt", "--output_language=ir", "-", "--",
                        "--enable_runtime=true"});
  auto expected_positional_args =
      MakeArgv({"spoor_opt", "-", "--enable_runtime=true"});
  const auto [_, positional_args] = ConfigFromCommandLineOrEnv(
      gsl::narrow_cast<int>(argv.size()), argv.data(), get_env);
  ASSERT_EQ(positional_args, expected_positional_args);
}

TEST(CommandLineConfig, Version) {  // NOLINT
  const auto get_env = [](const char* /*unused*/) -> const char* {
    return nullptr;
  };
  auto argv = MakeArgv({"spoor_opt", "--version"});
  EXPECT_EXIT(  // NOLINT
      ConfigFromCommandLineOrEnv(argv.size(), argv.data(), get_env),
      testing::ExitedWithCode(0), "");
}

TEST(CommandLineConfig, Help) {  // NOLINT
  const auto get_env = [](const char* /*unused*/) -> const char* {
    return nullptr;
  };
  auto argv = MakeArgv({"spoor_opt", "--help"});
  EXPECT_DEATH(  // NOLINT
      ConfigFromCommandLineOrEnv(argv.size(), argv.data(), get_env), "");
}

TEST(CommandLineConfig, UnknownFlag) {  // NOLINT
  const auto get_env = [](const char* /*unused*/) -> const char* {
    return nullptr;
  };
  auto argv = MakeArgv({"spoor_opt", "--unknown_flag"});
  EXPECT_DEATH(  // NOLINT
      ConfigFromCommandLineOrEnv(argv.size(), argv.data(), get_env),
      "Unknown command line flag 'unknown_flag'");
}

TEST(CommandLineconfig, AbslParseFlag) {  // NOLINT
  for (const auto& [key, value] : kOutputLanguages) {
    OutputLanguage language{};
    std::string error{};
    const auto success = AbslParseFlag(key, &language, &error);
    ASSERT_TRUE(success);
    ASSERT_TRUE(error.empty());
    ASSERT_EQ(language, value);
  }

  OutputLanguage language{};
  std::string error{};
  const auto success = AbslParseFlag("invalid", &language, &error);
  ASSERT_FALSE(success);
  ASSERT_EQ(error, "Unknown language 'invalid'. Options: bitcode, ir.");
}

TEST(CommandLineconfig, AbslUnparseFlag) {  // NOLINT
  for (const auto& [key, value] : kOutputLanguages) {
    const auto result = AbslUnparseFlag(value);
    ASSERT_EQ(result, key);
  }
  const auto result = AbslUnparseFlag(static_cast<OutputLanguage>(100));
  ASSERT_EQ(result, "100");
}

}  // namespace
