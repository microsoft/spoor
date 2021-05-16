// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/symbols/symbols_file_reader.h"

#include <filesystem>
#include <memory>
#include <sstream>

#include "gmock/gmock.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/util/time_util.h"
#include "gtest/gtest.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "symbols_reader.h"
#include "util/file_system/file_reader_mock.h"
#include "util/result.h"

namespace {

using google::protobuf::util::MessageDifferencer;
using google::protobuf::util::TimeUtil;
using spoor::instrumentation::symbols::Symbols;
using spoor::instrumentation::symbols::SymbolsFileReader;
using testing::Return;
using testing::ReturnRef;
using util::file_system::testing::FileReaderMock;
using Error = spoor::instrumentation::symbols::SymbolsReader::Error;

TEST(SymbolsFileReader, Read) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/file.spoor_symbols"};
  const auto expected_symbols = [] {
    Symbols symbols{};
    auto& function_symbols_table = *symbols.mutable_function_symbols_table();
    auto& function_infos = function_symbols_table[42];
    auto& function_info = *function_infos.add_function_infos();
    function_info.set_module_id("module_id");
    function_info.set_linkage_name("linkage_name");
    function_info.set_demangled_name("Demangled name");
    function_info.set_file_name("file.source");
    function_info.set_directory("/path/to/directory");
    function_info.set_line(42);
    function_info.set_instrumented(true);
    *function_info.mutable_created_at() = TimeUtil::NanosecondsToTimestamp(42);
    return symbols;
  }();
  std::stringstream buffer{};
  expected_symbols.SerializeToOstream(&buffer);
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());
  SymbolsFileReader symbols_file_reader{{
      .file_reader = std::move(file_reader),
  }};
  const auto result = symbols_file_reader.Read(file_path);
  ASSERT_TRUE(result.IsOk());
  const auto& symbols = result.Ok();
  ASSERT_TRUE(MessageDifferencer::Equals(symbols, expected_symbols));
}

TEST(SymbolsFileReader, ReadHandlesFailedToOpenFile) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/file.spoor_symbols"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(false));
  SymbolsFileReader symbols_file_reader{{
      .file_reader = std::move(file_reader),
  }};
  const auto result = symbols_file_reader.Read(file_path);
  ASSERT_TRUE(result.IsErr());
  ASSERT_EQ(result.Err(), Error::kFailedToOpenFile);
}

TEST(SymbolsFileReader, ReadHandlesCorruptData) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/file.spoor_symbols"};
  std::stringstream buffer{"bad data"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path, std::ios::binary));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  SymbolsFileReader symbols_file_reader{{
      .file_reader = std::move(file_reader),
  }};
  const auto result = symbols_file_reader.Read(file_path);
  ASSERT_TRUE(result.IsErr());
  ASSERT_EQ(result.Err(), Error::kCorruptData);
}

}  // namespace
