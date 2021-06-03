// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/symbols/symbols_file_writer.h"

#include <filesystem>
#include <ios>
#include <memory>
#include <sstream>

#include "gmock/gmock.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/util/time_util.h"
#include "gtest/gtest.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "util/file_system/file_writer_mock.h"
#include "util/result.h"

namespace {

using google::protobuf::util::MessageDifferencer;
using google::protobuf::util::TimeUtil;
using spoor::instrumentation::symbols::Symbols;
using spoor::instrumentation::symbols::SymbolsFileWriter;
using testing::Return;
using testing::ReturnRef;
using util::file_system::testing::FileWriterMock;
using Error = spoor::instrumentation::symbols::SymbolsWriter::Error;

TEST(SymbolsFileWriter, Write) {  // NOLINT
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
  auto file_writer = std::make_unique<FileWriterMock>();
  EXPECT_CALL(*file_writer, Open(file_path, std::ios::binary));
  EXPECT_CALL(*file_writer, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_writer, Ostream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_writer, Close());
  SymbolsFileWriter symbols_file_writer{
      {.file_writer = std::move(file_writer)}};
  const auto result = symbols_file_writer.Write(file_path, expected_symbols);
  ASSERT_TRUE(result.IsOk());
  Symbols symbols{};
  symbols.ParseFromIstream(&buffer);
  ASSERT_TRUE(MessageDifferencer::Equals(symbols, expected_symbols));
}

TEST(SymbolsFileWriter, WriteHandlesFailedToOpenFile) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/file.spoor_symbols"};
  auto file_writer = std::make_unique<FileWriterMock>();
  EXPECT_CALL(*file_writer, Open(file_path, std::ios::binary));
  EXPECT_CALL(*file_writer, IsOpen()).WillOnce(Return(false));
  SymbolsFileWriter symbols_file_writer{
      {.file_writer = std::move(file_writer)}};
  Symbols symbols{};
  const auto result = symbols_file_writer.Write(file_path, symbols);
  ASSERT_TRUE(result.IsErr());
  ASSERT_EQ(result.Err(), Error::kFailedToOpenFile);
}

TEST(SymbolsFileWriter, WriteHandlesSerializationError) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/file.spoor_symbols"};
  std::stringstream buffer{};
  buffer.setstate(std::ios::failbit);
  auto file_writer = std::make_unique<FileWriterMock>();
  EXPECT_CALL(*file_writer, Open(file_path, std::ios::binary));
  EXPECT_CALL(*file_writer, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_writer, Ostream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_writer, Close());
  SymbolsFileWriter symbols_file_writer{
      {.file_writer = std::move(file_writer)}};
  Symbols symbols{};
  const auto result = symbols_file_writer.Write(file_path, symbols);
  ASSERT_TRUE(result.IsErr());
  ASSERT_EQ(result.Err(), Error::kSerializationError);
}

}  // namespace
