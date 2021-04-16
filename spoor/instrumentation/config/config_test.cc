// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/config.h"

#include "gtest/gtest.h"

namespace {

using spoor::instrumentation::config::Config;
using spoor::instrumentation::config::OutputLanguage;

TEST(Config, ConfigEquality) {  // NOLINT
  const Config config_a{
      .enable_runtime = false,
      .force_binary_output = true,
      .function_allow_list_file = "/path/to/allow_list.txt",
      .function_blocklist_file = "/path/to/blocklist.txt",
      .initialize_runtime = false,
      .inject_instrumentation = false,
      .instrumented_function_map_output_path = "/path/to/output/",
      .min_instruction_threshold = 42,
      .module_id = "ModuleId",
      .output_file = "/path/to/output_file.ll",
      .output_language = OutputLanguage::kIr};
  const Config config_b{
      .enable_runtime = false,
      .force_binary_output = true,
      .function_allow_list_file = "/path/to/allow_list.txt",
      .function_blocklist_file = "/path/to/blocklist.txt",
      .initialize_runtime = false,
      .inject_instrumentation = false,
      .instrumented_function_map_output_path = "/path/to/output/",
      .min_instruction_threshold = 42,
      .module_id = "ModuleId",
      .output_file = "/path/to/output_file.ll",
      .output_language = OutputLanguage::kIr};
  const Config config_c{.enable_runtime = true,
                        .force_binary_output = false,
                        .function_allow_list_file = "",
                        .function_blocklist_file = "",
                        .initialize_runtime = true,
                        .inject_instrumentation = true,
                        .instrumented_function_map_output_path = "",
                        .min_instruction_threshold = 0,
                        .module_id = "",
                        .output_file = "",
                        .output_language = OutputLanguage::kBitcode};
  ASSERT_EQ(config_a, config_b);
  ASSERT_NE(config_a, config_c);
  ASSERT_NE(config_b, config_c);
}

TEST(Config, OutputLanguageBinaryOutput) {  // NOLINT
  ASSERT_TRUE(BinaryOutput(OutputLanguage::kBitcode));
  ASSERT_FALSE(BinaryOutput(OutputLanguage::kIr));
}

}  // namespace
