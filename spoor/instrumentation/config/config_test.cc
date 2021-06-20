// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/config.h"

#include "gtest/gtest.h"

namespace {

using spoor::instrumentation::config::Config;
using spoor::instrumentation::config::OutputLanguage;

TEST(Config, ConfigEquality) {  // NOLINT
  const Config config_a{.enable_runtime = false,
                        .filters_file = "/path/to/filters.toml",
                        .force_binary_output = true,
                        .initialize_runtime = false,
                        .inject_instrumentation = false,
                        .module_id = "ModuleId",
                        .output_file = "/path/to/output_file.ll",
                        .output_symbols_file = "/path/to/file.spoor_symbols",
                        .output_language = OutputLanguage::kIr};
  const Config config_b{.enable_runtime = false,
                        .filters_file = "/path/to/filters.toml",
                        .force_binary_output = true,
                        .initialize_runtime = false,
                        .inject_instrumentation = false,
                        .module_id = "ModuleId",
                        .output_file = "/path/to/output_file.ll",
                        .output_symbols_file = "/path/to/file.spoor_symbols",
                        .output_language = OutputLanguage::kIr};
  const Config config_c{.enable_runtime = true,
                        .filters_file = "",
                        .force_binary_output = false,
                        .initialize_runtime = true,
                        .inject_instrumentation = true,
                        .module_id = "",
                        .output_file = "",
                        .output_symbols_file = "",
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
