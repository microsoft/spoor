// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/filters/filters_file_reader.h"

#include <filesystem>
#include <memory>
#include <sstream>
#include <vector>

#include "absl/strings/str_format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "spoor/instrumentation/filters/filters.h"
#include "spoor/instrumentation/filters/filters_reader.h"
#include "util/file_system/file_reader_mock.h"

namespace {

using spoor::instrumentation::filters::Filter;
using spoor::instrumentation::filters::FiltersFileReader;
using spoor::instrumentation::filters::FiltersReader;
using spoor::instrumentation::filters::kActions;
using testing::Return;
using testing::ReturnRef;
using util::file_system::testing::FileReaderMock;

TEST(FiltersFileReader, ParsesConfig) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/filters.toml"};
  std::stringstream buffer{R"(
    [[allow]]
    rule_name = "Rule 0"
    source_file_path = ".*path/to/.*\\.extension"
    function_demangled_name = "^std::.*"
    function_linkage_name = "^_ZNKSt3.*"
    function_ir_instruction_count_lt = 400
    function_ir_instruction_count_gt = 200
    
    [[block]]
    rule_name = "Rule 1"
    source_file_path = ".*path/to/other/.*\\.extension"
    function_demangled_name = ".*[S|s]tring.*"
    function_linkage_name = "^_Z.*"
    function_ir_instruction_count_lt = 300
    function_ir_instruction_count_gt = 100
    
    [[allow]]
    rule_name = "Rule 2"
    function_demangled_name = "pattern"
    
    [[block]]
    rule_name = "Rule 3"
    function_linkage_name = "pattern"
    
    [[allow]]
    
    [[block]]
  )"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());
  FiltersFileReader filters_file_reader{{
      .file_reader = std::move(file_reader),
  }};
  const std::vector<Filter> expected_filters{
      {
          .action = Filter::Action::kAllow,
          .rule_name = "Rule 0",
          .source_file_path = ".*path/to/.*\\.extension",
          .function_demangled_name = "^std::.*",
          .function_linkage_name = "^_ZNKSt3.*",
          .function_ir_instruction_count_lt = 400,
          .function_ir_instruction_count_gt = 200,
      },
      {
          .action = Filter::Action::kAllow,
          .rule_name = "Rule 2",
          .source_file_path = std::nullopt,
          .function_demangled_name = "pattern",
          .function_linkage_name = std::nullopt,
          .function_ir_instruction_count_lt = std::nullopt,
          .function_ir_instruction_count_gt = std::nullopt,
      },
      {
          .action = Filter::Action::kAllow,
          .rule_name = std::nullopt,
          .source_file_path = std::nullopt,
          .function_demangled_name = std::nullopt,
          .function_linkage_name = std::nullopt,
          .function_ir_instruction_count_lt = std::nullopt,
          .function_ir_instruction_count_gt = std::nullopt,
      },
      {
          .action = Filter::Action::kBlock,
          .rule_name = "Rule 1",
          .source_file_path = ".*path/to/other/.*\\.extension",
          .function_demangled_name = ".*[S|s]tring.*",
          .function_linkage_name = "^_Z.*",
          .function_ir_instruction_count_lt = 300,
          .function_ir_instruction_count_gt = 100,
      },
      {
          .action = Filter::Action::kBlock,
          .rule_name = "Rule 3",
          .source_file_path = std::nullopt,
          .function_demangled_name = std::nullopt,
          .function_linkage_name = "pattern",
          .function_ir_instruction_count_lt = std::nullopt,
          .function_ir_instruction_count_gt = std::nullopt,
      },
      {
          .action = Filter::Action::kBlock,
          .rule_name = std::nullopt,
          .source_file_path = std::nullopt,
          .function_demangled_name = std::nullopt,
          .function_linkage_name = std::nullopt,
          .function_ir_instruction_count_lt = std::nullopt,
          .function_ir_instruction_count_gt = std::nullopt,
      },
  };
  const auto filters_result = filters_file_reader.Read(file_path);
  ASSERT_TRUE(filters_result.IsOk());
  const auto& parsed_filters = filters_result.Ok();
  ASSERT_EQ(parsed_filters, expected_filters);
}

TEST(FiltersFileReader, HandlesMalformedFile) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/filters.toml"};
  std::stringstream buffer{R"(
    [[allow]]
    rule_name = "Rule 0"
    source_file_path = ".*\.extension"
  )"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());
  FiltersFileReader filters_file_reader{{
      .file_reader = std::move(file_reader),
  }};
  const auto filters_result = filters_file_reader.Read(file_path);
  ASSERT_TRUE(filters_result.IsErr());
  const auto& [type, message] = filters_result.Err();
  ASSERT_EQ(type, FiltersReader::Error::Type::kMalformedFile);
  ASSERT_EQ(message,
            "Error while parsing string: unknown escape sequence '\\.'");
}

TEST(FiltersFileReader, HandlesUnknownTopLevelNode) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/filters.toml"};
  std::stringstream buffer{"bad_key = 42"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());
  FiltersFileReader filters_file_reader{{
      .file_reader = std::move(file_reader),
  }};
  const auto filters_result = filters_file_reader.Read(file_path);
  ASSERT_TRUE(filters_result.IsErr());
  const auto& [type, message] = filters_result.Err();
  ASSERT_EQ(type, FiltersReader::Error::Type::kUnknownNode);
  ASSERT_EQ(message, "Unknown filter type \"bad_key\".");
}

TEST(FiltersFileReader, HandlesMalformedFiltersNode) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/filters.toml"};
  for (const auto& node_values : {"[42]", "42"}) {
    for (const auto& [key, _] : kActions) {
      std::stringstream buffer{absl::StrFormat("%s = %s", key, node_values)};
      auto file_reader = std::make_unique<FileReaderMock>();
      EXPECT_CALL(*file_reader, Open(file_path));
      EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
      EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
      EXPECT_CALL(*file_reader, Close());
      FiltersFileReader filters_file_reader{{
          .file_reader = std::move(file_reader),
      }};
      const auto filters_result = filters_file_reader.Read(file_path);
      ASSERT_TRUE(filters_result.IsErr());
      const auto& [type, message] = filters_result.Err();
      ASSERT_EQ(type, FiltersReader::Error::Type::kMalformedNode);
      ASSERT_EQ(message,
                absl::StrFormat(
                    "Malformed node \"%s\" is not a list of filters.", key));
    }
  }
}

TEST(FiltersFileReader, HandlesMalformedFilterNode) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/filters.toml"};
  std::stringstream buffer{R"(
    [[allow]]
    rule_name = "Rule 0"
    bad_rule = "value"
  )"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());
  FiltersFileReader filters_file_reader{{
      .file_reader = std::move(file_reader),
  }};
  const auto filters_result = filters_file_reader.Read(file_path);
  ASSERT_TRUE(filters_result.IsErr());
  const auto& [type, message] = filters_result.Err();
  ASSERT_EQ(type, FiltersReader::Error::Type::kUnknownNode);
  ASSERT_EQ(message, "Unknown rule \"bad_rule\" in filter \"allow\".");
}

TEST(FiltersFileReader, HandlesOpenFileFailure) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/filters.toml"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(false));
  FiltersFileReader filters_file_reader{{
      .file_reader = std::move(file_reader),
  }};
  const auto filters_result = filters_file_reader.Read(file_path);
  ASSERT_TRUE(filters_result.IsErr());
  const auto& [type, message] = filters_result.Err();
  ASSERT_EQ(type, FiltersReader::Error::Type::kFailedToOpenFile);
  ASSERT_EQ(message,
            "Failed to open the file \"/path/to/filters.toml\" for reading.");
}

}  // namespace
