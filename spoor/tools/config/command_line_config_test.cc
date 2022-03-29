// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/tools/config/command_line_config.h"

#include <string_view>
#include <vector>

#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/tools/config/config.h"
#include "util/env/env.h"
#include "util/file_system/util.h"

namespace {

using spoor::tools::config::Config;
using spoor::tools::config::ConfigFromCommandLine;
using spoor::tools::config::kOutputFormats;
using spoor::tools::config::OutputFormat;
using util::file_system::PathExpansionOptions;

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
  const PathExpansionOptions path_expansion_options{
      .get_env = [](const char* key) -> const char* {
        if (key == nullptr) return nullptr;
        if (std::strncmp(key, util::env::kHomeKey.data(),
                         util::env::kHomeKey.size()) == 0) {
          return "/usr/you";
        }
        return nullptr;
      },
      .expand_tilde = true,
      .expand_environment_variables = true,
  };
  const Config expected_config{
      .output_file = "/usr/you/path/to/output_file.perfetto",
      .output_format = OutputFormat::kPerfettoProto,
  };
  auto argv = MakeArgv({"spoor", "--output_file=~/path/to/output_file.perfetto",
                        "--output_format=perfetto"});
  const auto expected_positional_args = MakeArgv({argv.front()});
  const auto [config, positional_args] = ConfigFromCommandLine(
      gsl::narrow_cast<int>(argv.size()), argv.data(), path_expansion_options);
  ASSERT_EQ(config, expected_config);
  ASSERT_EQ(positional_args, expected_positional_args);
}

TEST(CommandLineConfig, UsesDefaultValueWhenNotSpecified) {  // NOLINT
  const PathExpansionOptions path_expansion_options{
      .get_env = [](const char* /*unused*/) -> const char* { return nullptr; },
      .expand_tilde = true,
      .expand_environment_variables = true,
  };
  const Config expected_config{
      .output_file = "",
      .output_format = OutputFormat::kAutomatic,
  };
  auto argv = MakeArgv({"spoor"});
  const auto expected_positional_args = MakeArgv({argv.front()});
  const auto [config, positional_args] = ConfigFromCommandLine(
      gsl::narrow_cast<int>(argv.size()), argv.data(), path_expansion_options);
  ASSERT_EQ(config, expected_config);
  ASSERT_EQ(positional_args, expected_positional_args);
}

TEST(CommandLineConfig, PositionalArguments) {  // NOLINT
  const PathExpansionOptions path_expansion_options{
      .get_env = [](const char* /*unused*/) -> const char* { return nullptr; },
      .expand_tilde = true,
      .expand_environment_variables = true,
  };
  auto argv = MakeArgv({"spoor", "--output_format=spoor_symbols", "-", "--",
                        "--output_file=/path/to/output_file.spoor_symbols"});
  auto expected_positional_args = MakeArgv(
      {"spoor", "-", "--output_file=/path/to/output_file.spoor_symbols"});
  const auto [_, positional_args] = ConfigFromCommandLine(
      gsl::narrow_cast<int>(argv.size()), argv.data(), path_expansion_options);
  ASSERT_EQ(positional_args, expected_positional_args);
}

TEST(CommandLineConfig, Version) {  // NOLINT
  const PathExpansionOptions path_expansion_options{
      .get_env = [](const char* /*unused*/) -> const char* { return nullptr; },
      .expand_tilde = true,
      .expand_environment_variables = true,
  };
  auto argv = MakeArgv({"spoor", "--version"});
  EXPECT_EXIT(  // NOLINT
      ConfigFromCommandLine(argv.size(), argv.data(), path_expansion_options),
      testing::ExitedWithCode(0), "");
}

TEST(CommandLineConfig, Help) {  // NOLINT
  const PathExpansionOptions path_expansion_options{
      .get_env = [](const char* /*unused*/) -> const char* { return nullptr; },
      .expand_tilde = true,
      .expand_environment_variables = true,
  };
  auto argv = MakeArgv({"spoor", "--help"});
  EXPECT_DEATH(  // NOLINT
      ConfigFromCommandLine(argv.size(), argv.data(), path_expansion_options),
      "");
}

TEST(CommandLineConfig, UnknownFlag) {  // NOLINT
  const PathExpansionOptions path_expansion_options{
      .get_env = [](const char* /*unused*/) -> const char* { return nullptr; },
      .expand_tilde = true,
      .expand_environment_variables = true,
  };
  auto argv = MakeArgv({"spoor", "--unknown_flag"});
  EXPECT_DEATH(  // NOLINT
      ConfigFromCommandLine(argv.size(), argv.data(), path_expansion_options),
      "Unknown command line flag 'unknown_flag'");
}

TEST(CommandLineConfig, AbslParseFlag) {  // NOLINT
  for (const auto& [key, value] : kOutputFormats) {
    OutputFormat output_format{};
    std::string error{};
    const auto success = AbslParseFlag(key, &output_format, &error);
    ASSERT_TRUE(success);
    ASSERT_TRUE(error.empty());
    ASSERT_EQ(output_format, value);
  }

  OutputFormat output_format{};
  std::string error{};
  const auto success = AbslParseFlag("invalid", &output_format, &error);
  ASSERT_FALSE(success);
  ASSERT_EQ(error,
            "Unknown output format 'invalid'. Options: automatic, perfetto, "
            "spoor_symbols, csv.");
}

TEST(CommandLineConfig, AbslUnparseFlag) {  // NOLINT
  for (const auto& [key, value] : kOutputFormats) {
    const auto result = AbslUnparseFlag(value);
    ASSERT_EQ(result, key);
  }
  const auto result = AbslUnparseFlag(static_cast<OutputFormat>(100));
  ASSERT_EQ(result, "100");
}

}  // namespace
