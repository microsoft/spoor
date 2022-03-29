// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/config.h"

#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "spoor/instrumentation/config/output_language.h"
#include "spoor/instrumentation/config/source_mock.h"

namespace {

using spoor::instrumentation::config::Config;
using spoor::instrumentation::config::OutputLanguage;
using spoor::instrumentation::config::Source;
using spoor::instrumentation::config::testing::SourceMock;
using testing::Return;
using ReadError = spoor::instrumentation::config::Source::ReadError;

TEST(Config, Default) {  // NOLINT
  const Config expected_default_config{
      .enable_runtime = true,
      .filters_file = {},
      .force_binary_output = false,
      .initialize_runtime = true,
      .inject_instrumentation = true,
      .module_id = {},
      .output_file = "-",
      .output_language = OutputLanguage::kBitcode,
      .output_symbols_file = "",
  };
  const auto default_config = Config::Default();
  ASSERT_EQ(default_config, expected_default_config);
}

TEST(Config, ReadsFromSource) {  // NOLINT
  const Config default_config{
      .enable_runtime = true,
      .filters_file = {},
      .force_binary_output = false,
      .initialize_runtime = true,
      .inject_instrumentation = true,
      .module_id = {},
      .output_file = "-",
      .output_language = OutputLanguage::kBitcode,
      .output_symbols_file = "",
  };
  const Config expected_config{
      .enable_runtime = false,
      .filters_file = "/path/to/filters.toml",
      .force_binary_output = true,
      .initialize_runtime = false,
      .inject_instrumentation = false,
      .module_id = "ModuleId",
      .output_file = "/path/to/output.ll",
      .output_language = OutputLanguage::kIr,
      .output_symbols_file = "/path/to/symbols.spoor_symbols",
  };
  ASSERT_NE(default_config, expected_config);
  auto source_a = std::make_unique<SourceMock>();
  EXPECT_CALL(*source_a, Read()).WillOnce(Return(std::vector<ReadError>{}));
  EXPECT_CALL(*source_a, IsRead())
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*source_a, GetEnableRuntime())
      .WillOnce(Return(expected_config.enable_runtime));
  EXPECT_CALL(*source_a, GetFiltersFile())
      .WillOnce(Return(expected_config.filters_file));
  EXPECT_CALL(*source_a, GetForceBinaryOutput())
      .WillOnce(Return(expected_config.force_binary_output));
  EXPECT_CALL(*source_a, GetInitializeRuntime())
      .WillOnce(Return(expected_config.initialize_runtime));
  EXPECT_CALL(*source_a, GetInjectInstrumentation())
      .WillOnce(Return(expected_config.inject_instrumentation));
  EXPECT_CALL(*source_a, GetModuleId())
      .WillOnce(Return(expected_config.module_id));
  EXPECT_CALL(*source_a, GetOutputFile())
      .WillOnce(Return(expected_config.output_file));
  EXPECT_CALL(*source_a, GetOutputSymbolsFile())
      .WillOnce(Return(expected_config.output_symbols_file));
  EXPECT_CALL(*source_a, GetOutputLanguage())
      .WillOnce(Return(expected_config.output_language));
  auto source_b = std::make_unique<SourceMock>();
  auto source_c = std::make_unique<SourceMock>();

  std::vector<std::unique_ptr<Source>> sources{};
  sources.reserve(3);
  sources.emplace_back(std::move(source_a));
  sources.emplace_back(std::move(source_b));
  sources.emplace_back(std::move(source_c));
  const auto config =
      Config::FromSourcesOrDefault(std::move(sources), default_config);
  ASSERT_EQ(config, expected_config);
}

TEST(Config, ReadsFromSources) {  // NOLINT
  const Config default_config{
      .enable_runtime = true,
      .filters_file = {},
      .force_binary_output = false,
      .initialize_runtime = true,
      .inject_instrumentation = true,
      .module_id = {},
      .output_file = "-",
      .output_language = OutputLanguage::kBitcode,
      .output_symbols_file = "",
  };
  const Config expected_config{
      .enable_runtime = false,
      .filters_file = "/path/to/filters.toml",
      .force_binary_output = true,
      .initialize_runtime = false,
      .inject_instrumentation = false,
      .module_id = "ModuleId",
      .output_file = "/path/to/output.ll",
      .output_language = OutputLanguage::kIr,
      .output_symbols_file = "/path/to/symbols.spoor_symbols",
  };
  ASSERT_NE(default_config, expected_config);
  auto source_a = std::make_unique<SourceMock>();
  EXPECT_CALL(*source_a, Read()).WillOnce(Return(std::vector<ReadError>{}));
  EXPECT_CALL(*source_a, IsRead())
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*source_a, GetEnableRuntime()).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_a, GetFiltersFile()).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_a, GetForceBinaryOutput()).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_a, GetInitializeRuntime()).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_a, GetInjectInstrumentation())
      .WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_a, GetModuleId()).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_a, GetOutputFile()).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_a, GetOutputSymbolsFile()).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_a, GetOutputLanguage()).WillOnce(Return(std::nullopt));
  auto source_b = std::make_unique<SourceMock>();
  EXPECT_CALL(*source_b, Read()).WillOnce(Return(std::vector<ReadError>{}));
  EXPECT_CALL(*source_b, IsRead())
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*source_b, GetEnableRuntime())
      .WillOnce(Return(expected_config.enable_runtime));
  EXPECT_CALL(*source_b, GetFiltersFile())
      .WillOnce(Return(expected_config.filters_file));
  EXPECT_CALL(*source_b, GetForceBinaryOutput())
      .WillOnce(Return(expected_config.force_binary_output));
  EXPECT_CALL(*source_b, GetInitializeRuntime()).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_b, GetInjectInstrumentation())
      .WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_b, GetModuleId()).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_b, GetOutputFile()).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_b, GetOutputSymbolsFile()).WillOnce(Return(std::nullopt));
  EXPECT_CALL(*source_b, GetOutputLanguage()).WillOnce(Return(std::nullopt));
  auto source_c = std::make_unique<SourceMock>();
  EXPECT_CALL(*source_c, Read()).WillOnce(Return(std::vector<ReadError>{}));
  EXPECT_CALL(*source_c, IsRead())
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*source_c, GetEnableRuntime()).Times(0);
  EXPECT_CALL(*source_c, GetFiltersFile()).Times(0);
  EXPECT_CALL(*source_c, GetForceBinaryOutput()).Times(0);
  EXPECT_CALL(*source_c, GetInitializeRuntime())
      .WillOnce(Return(expected_config.initialize_runtime));
  EXPECT_CALL(*source_c, GetInjectInstrumentation())
      .WillOnce(Return(expected_config.inject_instrumentation));
  EXPECT_CALL(*source_c, GetModuleId())
      .WillOnce(Return(expected_config.module_id));
  EXPECT_CALL(*source_c, GetOutputFile())
      .WillOnce(Return(expected_config.output_file));
  EXPECT_CALL(*source_c, GetOutputSymbolsFile())
      .WillOnce(Return(expected_config.output_symbols_file));
  EXPECT_CALL(*source_c, GetOutputLanguage())
      .WillOnce(Return(expected_config.output_language));

  std::vector<std::unique_ptr<Source>> sources{};
  sources.reserve(3);
  sources.emplace_back(std::move(source_a));
  sources.emplace_back(std::move(source_b));
  sources.emplace_back(std::move(source_c));
  const auto config =
      Config::FromSourcesOrDefault(std::move(sources), default_config);
  ASSERT_EQ(config, expected_config);
}

TEST(Config, IgnoresErrors) {  // NOLINT
  const Config default_config{
      .enable_runtime = true,
      .filters_file = {},
      .force_binary_output = false,
      .initialize_runtime = true,
      .inject_instrumentation = true,
      .module_id = {},
      .output_file = "-",
      .output_language = OutputLanguage::kBitcode,
      .output_symbols_file = "",
  };
  const Config expected_config{
      .enable_runtime = false,
      .filters_file = "/path/to/filters.toml",
      .force_binary_output = true,
      .initialize_runtime = false,
      .inject_instrumentation = false,
      .module_id = "ModuleId",
      .output_file = "/path/to/output.ll",
      .output_language = OutputLanguage::kIr,
      .output_symbols_file = "/path/to/symbols.spoor_symbols",
  };
  ASSERT_NE(default_config, expected_config);
  auto source_a = std::make_unique<SourceMock>();
  EXPECT_CALL(*source_a, Read())
      .WillOnce(Return(std::vector<ReadError>{{
          .type = ReadError::Type::kUnknownKey,
          .message = "Error message",
      }}));
  EXPECT_CALL(*source_a, IsRead())
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*source_a, GetEnableRuntime())
      .WillOnce(Return(expected_config.enable_runtime));
  EXPECT_CALL(*source_a, GetFiltersFile())
      .WillOnce(Return(expected_config.filters_file));
  EXPECT_CALL(*source_a, GetForceBinaryOutput())
      .WillOnce(Return(expected_config.force_binary_output));
  EXPECT_CALL(*source_a, GetInitializeRuntime())
      .WillOnce(Return(expected_config.initialize_runtime));
  EXPECT_CALL(*source_a, GetInjectInstrumentation())
      .WillOnce(Return(expected_config.inject_instrumentation));
  EXPECT_CALL(*source_a, GetModuleId())
      .WillOnce(Return(expected_config.module_id));
  EXPECT_CALL(*source_a, GetOutputFile())
      .WillOnce(Return(expected_config.output_file));
  EXPECT_CALL(*source_a, GetOutputSymbolsFile())
      .WillOnce(Return(expected_config.output_symbols_file));
  EXPECT_CALL(*source_a, GetOutputLanguage())
      .WillOnce(Return(expected_config.output_language));

  std::vector<std::unique_ptr<Source>> sources{};
  sources.reserve(1);
  sources.emplace_back(std::move(source_a));
  const auto config =
      Config::FromSourcesOrDefault(std::move(sources), default_config);
  ASSERT_EQ(config, expected_config);
}

TEST(Config, ConfigEquality) {  // NOLINT
  const Config config_a{
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
  const Config config_b{
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
  const Config config_c{
      .enable_runtime = true,
      .filters_file = "",
      .force_binary_output = false,
      .initialize_runtime = true,
      .inject_instrumentation = true,
      .module_id = "",
      .output_file = "",
      .output_language = OutputLanguage::kBitcode,
      .output_symbols_file = "",
  };
  ASSERT_EQ(config_a, config_b);
  ASSERT_NE(config_a, config_c);
  ASSERT_NE(config_b, config_c);
}

}  // namespace
