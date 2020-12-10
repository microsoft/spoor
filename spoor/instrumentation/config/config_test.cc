// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/config.h"

#include <limits>
#include <unordered_map>

#include "gtest/gtest.h"
#include "util/numeric.h"

namespace {

using spoor::instrumentation::config::Config;
using spoor::instrumentation::config::kEnableRuntimeKey;
using spoor::instrumentation::config::kFunctionAllowListFileKey;
using spoor::instrumentation::config::kFunctionBlocklistFileKey;
using spoor::instrumentation::config::kInitializeRuntimeKey;
using spoor::instrumentation::config::kInstrumentationMapOutputPathKey;
using spoor::instrumentation::config::kMinInstructionThresholdKey;

TEST(Config, GetsUserProvidedValue) {  // NOLINT
  const auto get_env = [](const char* key) {
    const std::unordered_map<std::string_view, std::string_view> environment{
        {kInstrumentationMapOutputPathKey, "/path/to/output/"},
        {kInitializeRuntimeKey, "false"},
        {kEnableRuntimeKey, "false"},
        {kMinInstructionThresholdKey, "42"},
        {kFunctionAllowListFileKey, "/path/to/allow_list.txt"},
        {kFunctionBlocklistFileKey, "/path/to/blocklist.txt"}};
    return environment.at(key).data();
  };
  const Config expected_options{
      .instrumentation_map_output_path = "/path/to/output/",
      .initialize_runtime = false,
      .enable_runtime = false,
      .min_instruction_threshold = 42,
      .function_allow_list_file = "/path/to/allow_list.txt",
      .function_blocklist_file = "/path/to/blocklist.txt"};
  ASSERT_EQ(Config::FromEnv(get_env), expected_options);
}

TEST(Config, UsesDefaultValueWhenNotSpecified) {  // NOLINT
  const auto get_env = [](const char* /*unused*/) -> const char* {
    return nullptr;
  };
  const Config expected_options{.instrumentation_map_output_path = ".",
                                .initialize_runtime = true,
                                .enable_runtime = true,
                                .min_instruction_threshold = 0,
                                .function_allow_list_file = {},
                                .function_blocklist_file = {}};
  ASSERT_EQ(Config::FromEnv(get_env), expected_options);
}

}  // namespace
