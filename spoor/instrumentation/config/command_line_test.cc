// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/command_line.h"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <string_view>
#include <vector>

#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/instrumentation/config/config.h"
#include "spoor/instrumentation/config/output_language.h"
#include "spoor/instrumentation/config/source.h"
#include "util/env/env.h"
#include "util/file_system/util.h"
#include "util/flat_map/flat_map.h"

namespace {

using spoor::instrumentation::config::Config;
using spoor::instrumentation::config::ConfigFromCommandLineOrDefault;
using spoor::instrumentation::config::kOutputLanguages;
using spoor::instrumentation::config::OutputLanguage;

auto PathExpansionOptions() -> util::file_system::PathExpansionOptions {
  auto get_env = [](const char* key) -> const char* {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 2>
        environment{
            {util::env::kHomeKey, "/usr/you"},
            {"FOO", "bar"},
        };
    auto value = environment.FirstValueForKey(key);
    if (value.has_value()) return value.value().data();
    return nullptr;
  };
  return {
      .get_env = get_env,
      .expand_tilde = true,
      .expand_environment_variables = true,
  };
}

auto MakeArgv(const std::vector<std::string_view>& args) -> std::vector<char*> {
  std::vector<char*> argv{};
  argv.reserve(args.size());
  std::transform(std::cbegin(args), std::cend(args), std::back_inserter(argv),
                 [](const auto string_view) {
                   // `absl::ParseCommandLine` requires a (non-const) char** as
                   // an argument.
                   // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
                   return const_cast<char*>(string_view.data());
                 });
  return argv;
}

TEST(CommandLineConfig, ParsesCommandLine) {  // NOLINT
  const Config expected_config{
      .enable_runtime = false,
      .filters_file = "/path/to/filters.toml",
      .force_binary_output = true,
      .initialize_runtime = false,
      .inject_instrumentation = false,
      .module_id = "ModuleId",
      .output_file = "/path/to/output_file.ll",
      .output_language = OutputLanguage::kIr,
      .output_symbols_file = "/path/to/file.spoor_symbols",
  };
  auto argv = MakeArgv({
      "spoor_opt",
      "--enable_runtime=false",
      "--filters_file=/path/to/filters.toml",
      "--force_binary_output=true",
      "--initialize_runtime=false",
      "--inject_instrumentation=false",
      "--module_id=ModuleId",
      "--output_file=/path/to/output_file.ll",
      "--output_symbols_file=/path/to/file.spoor_symbols",
      "--output_language=ir",
  });
  const auto expected_positional_args = MakeArgv({argv.front()});
  const auto [config, positional_args] = ConfigFromCommandLineOrDefault(
      gsl::narrow_cast<int>(argv.size()), argv.data(), Config::Default(),
      PathExpansionOptions());
  ASSERT_EQ(config, expected_config);
  ASSERT_EQ(positional_args, expected_positional_args);
}

TEST(CommandLineConfig, ExpandsPaths) {  // NOLINT
  const Config expected_config{
      .enable_runtime = false,
      .filters_file = "/usr/you/bar/filters.toml",
      .force_binary_output = true,
      .initialize_runtime = false,
      .inject_instrumentation = false,
      .module_id = "ModuleId",
      .output_file = "/usr/you/bar/output_file.ll",
      .output_language = OutputLanguage::kIr,
      .output_symbols_file = "/usr/you/bar/file.spoor_symbols",
  };
  auto argv = MakeArgv({
      "spoor_opt",
      "--enable_runtime=false",
      "--filters_file=~/$FOO/filters.toml",
      "--force_binary_output=true",
      "--initialize_runtime=false",
      "--inject_instrumentation=false",
      "--module_id=ModuleId",
      "--output_file=~/$FOO/output_file.ll",
      "--output_symbols_file=~/$FOO/file.spoor_symbols",
      "--output_language=ir",
  });
  const auto expected_positional_args = MakeArgv({argv.front()});
  const auto [config, positional_args] = ConfigFromCommandLineOrDefault(
      gsl::narrow_cast<int>(argv.size()), argv.data(), Config::Default(),
      PathExpansionOptions());
  ASSERT_EQ(config, expected_config);
  ASSERT_EQ(positional_args, expected_positional_args);
}

TEST(CommandLineConfig, UsesDefaultValueWhenNotSpecified) {  // NOLINT
  const auto expected_config = Config::Default();
  auto argv = MakeArgv({"spoor_opt"});
  const auto [config, positional_args] = ConfigFromCommandLineOrDefault(
      gsl::narrow_cast<int>(argv.size()), argv.data(), expected_config,
      PathExpansionOptions());
  ASSERT_EQ(config, expected_config);
  ASSERT_EQ(positional_args, argv);
}

TEST(CommandLineConfig, PositionalArguments) {  // NOLINT
  auto argv = MakeArgv({
      "spoor_opt",
      "-",
      "--output_language=ir",
      "--",
      "--enable_runtime=true",
  });
  auto expected_positional_args =
      MakeArgv({"spoor_opt", "-", "--enable_runtime=true"});
  const auto [_, positional_args] = ConfigFromCommandLineOrDefault(
      gsl::narrow_cast<int>(argv.size()), argv.data(), Config::Default(),
      PathExpansionOptions());
  ASSERT_EQ(positional_args, expected_positional_args);
}

TEST(CommandLineConfig, Version) {  // NOLINT
  auto argv = MakeArgv({"spoor_opt", "--version"});
  EXPECT_EXIT(  // NOLINT
      ConfigFromCommandLineOrDefault(argv.size(), argv.data(),
                                     Config::Default(), PathExpansionOptions()),
      testing::ExitedWithCode(0), "");
}

TEST(CommandLineConfig, Help) {  // NOLINT
  auto argv = MakeArgv({"spoor_opt", "--help"});
  EXPECT_DEATH(  // NOLINT
      ConfigFromCommandLineOrDefault(argv.size(), argv.data(),
                                     Config::Default(), PathExpansionOptions()),
      "");
}

TEST(CommandLineConfig, UnknownFlag) {  // NOLINT
  auto argv = MakeArgv({"spoor_opt", "--unknown_flag"});
  EXPECT_DEATH(  // NOLINT
      ConfigFromCommandLineOrDefault(argv.size(), argv.data(),
                                     Config::Default(), PathExpansionOptions()),
      "Unknown command line flag 'unknown_flag'");
}

TEST(CommandLineConfig, AbslParseFlag) {  // NOLINT
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

TEST(CommandLineConfig, AbslUnparseFlag) {  // NOLINT
  for (const auto& [key, value] : kOutputLanguages) {
    const auto result = AbslUnparseFlag(value);
    ASSERT_EQ(result, key);
  }
  const auto result = AbslUnparseFlag(static_cast<OutputLanguage>(100));
  ASSERT_EQ(result, "100");
}

}  // namespace
