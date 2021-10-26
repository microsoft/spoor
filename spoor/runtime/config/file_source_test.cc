// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/config/file_source.h"

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_buffer.h"
#include "spoor/runtime/config/source.h"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor.h"
#include "util/file_system/file_reader_mock.h"

namespace {

using spoor::runtime::config::FileSource;
using spoor::runtime::config::kCompressionStrategyFileKey;
using spoor::runtime::config::kDynamicEventPoolCapacityFileKey;
using spoor::runtime::config::kDynamicEventSliceBorrowCasAttemptsFileKey;
using spoor::runtime::config::kEventBufferRetentionDurationNanosecondsFileKey;
using spoor::runtime::config::kFileConfigKeys;
using spoor::runtime::config::kFlushAllEventsFileKey;
using spoor::runtime::config::kMaxDynamicEventBufferSliceCapacityFileKey;
using spoor::runtime::config::kMaxFlushBufferToFileAttemptsFileKey;
using spoor::runtime::config::kMaxReservedEventBufferSliceCapacityFileKey;
using spoor::runtime::config::kReservedEventPoolCapacityFileKey;
using spoor::runtime::config::kSessionIdFileKey;
using spoor::runtime::config::kThreadEventBufferCapacityFileKey;
using spoor::runtime::config::kTraceFilePathFileKey;
using spoor::runtime::config::Source;
using testing::Return;
using testing::ReturnRef;
using util::file_system::testing::FileReaderMock;
using SizeType = spoor::runtime::buffer::CircularBuffer<
    spoor::runtime::trace::Event>::SizeType;

TEST(FileSource, ReadsConfig) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/spoor_runtime_config.toml"};
  std::stringstream buffer{};
  buffer << kTraceFilePathFileKey << " = '/path/to/trace/'\n"
         << kCompressionStrategyFileKey << " = 'snappy'\n"
         << kSessionIdFileKey << " = 1\n"
         << kThreadEventBufferCapacityFileKey << " = 2\n"
         << kMaxReservedEventBufferSliceCapacityFileKey << " = 3\n"
         << kMaxDynamicEventBufferSliceCapacityFileKey << " = 4\n"
         << kReservedEventPoolCapacityFileKey << " = 5\n"
         << kDynamicEventPoolCapacityFileKey << " = 6\n"
         << kDynamicEventSliceBorrowCasAttemptsFileKey << " = 7\n"
         << kEventBufferRetentionDurationNanosecondsFileKey << " = 8\n"
         << kMaxFlushBufferToFileAttemptsFileKey << " = 9\n"
         << kFlushAllEventsFileKey << " = true";
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());
  FileSource file_config{{
      .file_reader{std::move(file_reader)},
      .file_path{file_path},
  }};

  ASSERT_FALSE(file_config.TraceFilePath().has_value());
  ASSERT_FALSE(file_config.CompressionStrategy().has_value());
  ASSERT_FALSE(file_config.SessionId().has_value());
  ASSERT_FALSE(file_config.ThreadEventBufferCapacity().has_value());
  ASSERT_FALSE(file_config.MaxReservedEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(file_config.MaxDynamicEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(file_config.ReservedEventPoolCapacity().has_value());
  ASSERT_FALSE(file_config.DynamicEventPoolCapacity().has_value());
  ASSERT_FALSE(file_config.DynamicEventSliceBorrowCasAttempts().has_value());
  ASSERT_FALSE(
      file_config.EventBufferRetentionDurationNanoseconds().has_value());
  ASSERT_FALSE(file_config.MaxFlushBufferToFileAttempts().has_value());
  ASSERT_FALSE(file_config.FlushAllEvents().has_value());

  ASSERT_FALSE(file_config.IsRead());
  const auto read_errors = file_config.Read();
  ASSERT_TRUE(file_config.IsRead());
  ASSERT_TRUE(read_errors.empty());

  const auto trace_file_path = file_config.TraceFilePath();
  ASSERT_TRUE(trace_file_path.has_value());
  ASSERT_EQ(trace_file_path.value(), "/path/to/trace/");
  const auto compression_strategy = file_config.CompressionStrategy();
  ASSERT_TRUE(compression_strategy.has_value());
  ASSERT_EQ(compression_strategy.value(), util::compression::Strategy::kSnappy);
  const auto session_id = file_config.SessionId();
  ASSERT_TRUE(session_id.has_value());
  ASSERT_EQ(session_id.value(), 1);
  const auto thread_event_buffer_capacity =
      file_config.ThreadEventBufferCapacity();
  ASSERT_TRUE(thread_event_buffer_capacity.has_value());
  ASSERT_EQ(thread_event_buffer_capacity.value(), 2);
  const auto max_reserved_event_buffer_slice_capacity =
      file_config.MaxReservedEventBufferSliceCapacity();
  ASSERT_TRUE(max_reserved_event_buffer_slice_capacity.has_value());
  ASSERT_EQ(max_reserved_event_buffer_slice_capacity.value(), 3);
  const auto max_dynamic_event_buffer_slice_capacity =
      file_config.MaxDynamicEventBufferSliceCapacity();
  ASSERT_TRUE(max_dynamic_event_buffer_slice_capacity.has_value());
  ASSERT_EQ(max_dynamic_event_buffer_slice_capacity.value(), 4);
  const auto reserved_event_pool_capacity =
      file_config.ReservedEventPoolCapacity();
  ASSERT_TRUE(reserved_event_pool_capacity.has_value());
  ASSERT_EQ(reserved_event_pool_capacity.value(), 5);
  const auto dynamic_event_pool_capacity =
      file_config.DynamicEventPoolCapacity();
  ASSERT_TRUE(dynamic_event_pool_capacity.has_value());
  ASSERT_EQ(dynamic_event_pool_capacity.value(), 6);
  const auto dynamic_event_slice_borrow_cas_attempts =
      file_config.DynamicEventSliceBorrowCasAttempts();
  ASSERT_TRUE(dynamic_event_slice_borrow_cas_attempts.has_value());
  ASSERT_EQ(dynamic_event_slice_borrow_cas_attempts.value(), 7);
  const auto event_buffer_retention_duration_nanoseconds =
      file_config.EventBufferRetentionDurationNanoseconds();
  ASSERT_TRUE(event_buffer_retention_duration_nanoseconds.has_value());
  ASSERT_EQ(event_buffer_retention_duration_nanoseconds, 8);
  const auto max_flush_buffer_to_file_attempts =
      file_config.MaxFlushBufferToFileAttempts();
  ASSERT_TRUE(max_flush_buffer_to_file_attempts.has_value());
  ASSERT_EQ(max_flush_buffer_to_file_attempts.value(), 9);
  const auto flush_all_events = file_config.FlushAllEvents();
  ASSERT_TRUE(flush_all_events.has_value());
  ASSERT_TRUE(flush_all_events.value());
}

TEST(FileSource, NormalizesCompressionStrategy) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/spoor_runtime_config.toml"};
  std::stringstream buffer{};
  buffer << kCompressionStrategyFileKey << " = \"     sNaPpY    \"";
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());
  FileSource file_config{{
      .file_reader{std::move(file_reader)},
      .file_path{file_path},
  }};

  const auto read_errors = file_config.Read();
  ASSERT_TRUE(read_errors.empty());

  const auto compression_strategy = file_config.CompressionStrategy();
  ASSERT_TRUE(compression_strategy.has_value());
  ASSERT_EQ(compression_strategy.value(), util::compression::Strategy::kSnappy);
}

TEST(FileSource, HandlesEmptyFile) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/spoor_runtime_config.toml"};
  std::stringstream buffer{};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());
  FileSource file_config{{
      .file_reader{std::move(file_reader)},
      .file_path{file_path},
  }};

  const auto read_errors = file_config.Read();
  ASSERT_TRUE(read_errors.empty());

  ASSERT_FALSE(file_config.TraceFilePath().has_value());
  ASSERT_FALSE(file_config.CompressionStrategy().has_value());
  ASSERT_FALSE(file_config.SessionId().has_value());
  ASSERT_FALSE(file_config.ThreadEventBufferCapacity().has_value());
  ASSERT_FALSE(file_config.MaxReservedEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(file_config.MaxDynamicEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(file_config.ReservedEventPoolCapacity().has_value());
  ASSERT_FALSE(file_config.DynamicEventPoolCapacity().has_value());
  ASSERT_FALSE(file_config.DynamicEventSliceBorrowCasAttempts().has_value());
  ASSERT_FALSE(
      file_config.EventBufferRetentionDurationNanoseconds().has_value());
  ASSERT_FALSE(file_config.MaxFlushBufferToFileAttempts().has_value());
  ASSERT_FALSE(file_config.FlushAllEvents().has_value());
}

TEST(FileSource, HandlesFailedToOpenFile) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/spoor_runtime_config.toml"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(false));
  EXPECT_CALL(*file_reader, Close()).Times(0);
  FileSource file_config{{
      .file_reader{std::move(file_reader)},
      .file_path{file_path},
  }};
  const auto read_errors = file_config.Read();
  ASSERT_EQ(read_errors.size(), 1);
  const auto& error = read_errors.front();
  ASSERT_EQ(error.type, Source::ReadError::Type::kFailedToOpenFile);
  ASSERT_EQ(error.message,
            "Failed to open the file \"/path/to/spoor_runtime_config.toml\" "
            "for reading.");
  ASSERT_TRUE(file_config.IsRead());
}

TEST(FileSource, HandlesMalformedFile) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/spoor_runtime_config.toml"};
  std::stringstream buffer{"bad toml"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());
  FileSource file_config{{
      .file_reader{std::move(file_reader)},
      .file_path{file_path},
  }};

  const auto read_errors = file_config.Read();
  ASSERT_EQ(read_errors.size(), 1);
  const auto& error = read_errors.front();
  ASSERT_EQ(error.type, Source::ReadError::Type::kMalformedFile);
}

TEST(FileSource, HandlesUnknownKey) {  // NOLINT
  constexpr std::string_view bad_key{"bad_key"};
  const std::filesystem::path file_path{"/path/to/spoor_runtime_config.toml"};
  std::stringstream buffer{};
  buffer << bad_key << " = false\n" << kFlushAllEventsFileKey << " = true";
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());
  FileSource file_config{{
      .file_reader{std::move(file_reader)},
      .file_path{file_path},
  }};

  const auto read_errors = file_config.Read();
  ASSERT_EQ(read_errors.size(), 1);
  const auto& error = read_errors.front();
  ASSERT_EQ(error.type, Source::ReadError::Type::kUnknownKey);
  ASSERT_EQ(error.message, absl::StrFormat("Unknown key \"%s\".", bad_key));

  const auto flush_all_events = file_config.FlushAllEvents();
  ASSERT_TRUE(flush_all_events.has_value());
  ASSERT_TRUE(flush_all_events.value());
}

TEST(FileSource, HandlesBadValue) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/spoor_runtime_config.toml"};
  std::stringstream buffer{};
  buffer << kTraceFilePathFileKey << " = 42\n"
         << kCompressionStrategyFileKey << " = 42\n"
         << kSessionIdFileKey << " = '42'\n"
         << kThreadEventBufferCapacityFileKey << " = '42'\n"
         << kMaxReservedEventBufferSliceCapacityFileKey << " = '42'\n"
         << kMaxDynamicEventBufferSliceCapacityFileKey << " = '42'\n"
         << kReservedEventPoolCapacityFileKey << " = '42'\n"
         << kDynamicEventPoolCapacityFileKey << " = '42'\n"
         << kDynamicEventSliceBorrowCasAttemptsFileKey << " = '42'\n"
         << kEventBufferRetentionDurationNanosecondsFileKey << " = '42'\n"
         << kMaxFlushBufferToFileAttemptsFileKey << " = '42'\n"
         << kFlushAllEventsFileKey << " = 'true'";
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());

  FileSource file_config{{
      .file_reader{std::move(file_reader)},
      .file_path{file_path},
  }};

  const auto read_errors = file_config.Read();
  ASSERT_EQ(read_errors.size(), std::size(kFileConfigKeys));
  ASSERT_TRUE(std::all_of(
      std::cbegin(read_errors), std::cend(read_errors), [](const auto& error) {
        return error.type == Source::ReadError::Type::kUnknownValue;
      }));
  const auto error_messages = [&read_errors] {
    std::vector<std::string> messages{};
    std::transform(std::cbegin(read_errors), std::cend(read_errors),
                   std::back_inserter(messages),
                   [](const auto& error) { return error.message; });
    std::sort(std::begin(messages), std::end(messages));
    return messages;
  }();
  const auto expected_error_messages = [&] {
    std::vector<std::string> messages{};
    std::transform(std::cbegin(kFileConfigKeys), std::cend(kFileConfigKeys),
                   std::back_inserter(messages), [](const auto key) {
                     return absl::StrFormat(
                         "Cannot parse value for key \"%s\".", key);
                   });
    std::sort(std::begin(messages), std::end(messages));
    return messages;
  }();
  ASSERT_EQ(error_messages, expected_error_messages);

  ASSERT_FALSE(file_config.TraceFilePath().has_value());
  ASSERT_FALSE(file_config.CompressionStrategy().has_value());
  ASSERT_FALSE(file_config.SessionId().has_value());
  ASSERT_FALSE(file_config.ThreadEventBufferCapacity().has_value());
  ASSERT_FALSE(file_config.MaxReservedEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(file_config.MaxDynamicEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(file_config.ReservedEventPoolCapacity().has_value());
  ASSERT_FALSE(file_config.DynamicEventPoolCapacity().has_value());
  ASSERT_FALSE(file_config.DynamicEventSliceBorrowCasAttempts().has_value());
  ASSERT_FALSE(
      file_config.EventBufferRetentionDurationNanoseconds().has_value());
  ASSERT_FALSE(file_config.MaxFlushBufferToFileAttempts().has_value());
  ASSERT_FALSE(file_config.FlushAllEvents().has_value());
}

TEST(FileSource, HandlesUnknownCompressionStrategy) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/spoor_runtime_config.toml"};
  std::stringstream buffer{};
  buffer << kCompressionStrategyFileKey << " = 'lz4'";

  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());
  FileSource file_config{{
      .file_reader{std::move(file_reader)},
      .file_path{file_path},
  }};

  const auto read_errors = file_config.Read();
  ASSERT_EQ(read_errors.size(), 1);
  const auto& error = read_errors.front();
  ASSERT_EQ(error.type, Source::ReadError::Type::kUnknownValue);
  ASSERT_EQ(error.message, "Unknown compression strategy \"lz4\".");
  const auto compression_strategy = file_config.CompressionStrategy();
  ASSERT_FALSE(compression_strategy.has_value());
}

}  // namespace
