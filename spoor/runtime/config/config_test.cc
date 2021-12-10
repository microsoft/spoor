// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/config/config.h"

#include <cstring>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "spoor/runtime/config/source.h"
#include "spoor/runtime/config/source_mock.h"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor.h"

namespace {

using spoor::runtime::config::Config;
using spoor::runtime::config::Source;
using spoor::runtime::config::testing::SourceMock;
using spoor::runtime::trace::SessionId;
using testing::Return;

TEST(Config, Default) {  // NOLINT
  constexpr SessionId session_id{42};
  const auto default_config = [] {
    auto config = Config::Default();
    // `session_id` is randomly generated at runtime which is difficult to test.
    config.session_id = session_id;
    return config;
  }();
  const Config expected_default_config{
      .trace_file_path = ".",
      .compression_strategy = util::compression::Strategy::kSnappy,
      .session_id = session_id,
      .thread_event_buffer_capacity = 10'000,
      .max_reserved_event_buffer_slice_capacity = 1'000,
      .max_dynamic_event_buffer_slice_capacity = 1'000,
      .reserved_event_pool_capacity = 0,
      .dynamic_event_pool_capacity = 0xffffffffffffffff,
      .dynamic_event_slice_borrow_cas_attempts = 1,
      .event_buffer_retention_duration_nanoseconds = 0,
      .max_flush_buffer_to_file_attempts = 2,
      .flush_all_events = true,
  };
}

TEST(Config, UsesDefaultConfigWithNoSource) {  // NOLINT
  const auto default_config = Config::Default();
  const auto get_env = [](const char* /*unused*/) { return nullptr; };
  const auto config = Config::FromSourcesOrDefault({}, default_config, get_env);
  ASSERT_EQ(config, default_config);
}

TEST(Config, ConfiguresFromSource) {  // NOLINT
  const Config default_config{
      .trace_file_path = "/path/to/trace/",
      .compression_strategy = util::compression::Strategy::kNone,
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = false,
  };
  const Config expected_config{
      .trace_file_path = "/path/to/other/trace/",
      .compression_strategy = util::compression::Strategy::kSnappy,
      .session_id = 10,
      .thread_event_buffer_capacity = 20,
      .max_reserved_event_buffer_slice_capacity = 30,
      .max_dynamic_event_buffer_slice_capacity = 40,
      .reserved_event_pool_capacity = 50,
      .dynamic_event_pool_capacity = 60,
      .dynamic_event_slice_borrow_cas_attempts = 70,
      .event_buffer_retention_duration_nanoseconds = 80,
      .max_flush_buffer_to_file_attempts = 90,
      .flush_all_events = true,
  };
  ASSERT_NE(default_config, expected_config);

  auto source = std::make_unique<SourceMock>();
  EXPECT_CALL(*source, Read())
      .WillOnce(Return(std::vector<Source::ReadError>{}));
  EXPECT_CALL(*source, IsRead())
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*source, TraceFilePath())
      .WillRepeatedly(Return(expected_config.trace_file_path));
  EXPECT_CALL(*source, CompressionStrategy())
      .WillRepeatedly(Return(expected_config.compression_strategy));
  EXPECT_CALL(*source, SessionId())
      .WillRepeatedly(Return(expected_config.session_id));
  EXPECT_CALL(*source, ThreadEventBufferCapacity())
      .WillRepeatedly(Return(expected_config.thread_event_buffer_capacity));
  EXPECT_CALL(*source, MaxReservedEventBufferSliceCapacity())
      .WillRepeatedly(
          Return(expected_config.max_reserved_event_buffer_slice_capacity));
  EXPECT_CALL(*source, MaxDynamicEventBufferSliceCapacity())
      .WillRepeatedly(
          Return(expected_config.max_dynamic_event_buffer_slice_capacity));
  EXPECT_CALL(*source, ReservedEventPoolCapacity())
      .WillRepeatedly(Return(expected_config.reserved_event_pool_capacity));
  EXPECT_CALL(*source, DynamicEventPoolCapacity())
      .WillRepeatedly(Return(expected_config.dynamic_event_pool_capacity));
  EXPECT_CALL(*source, DynamicEventSliceBorrowCasAttempts())
      .WillRepeatedly(
          Return(expected_config.dynamic_event_slice_borrow_cas_attempts));
  EXPECT_CALL(*source, EventBufferRetentionDurationNanoseconds())
      .WillRepeatedly(
          Return(expected_config.event_buffer_retention_duration_nanoseconds));
  EXPECT_CALL(*source, MaxFlushBufferToFileAttempts())
      .WillRepeatedly(
          Return(expected_config.max_flush_buffer_to_file_attempts));
  EXPECT_CALL(*source, FlushAllEvents())
      .WillRepeatedly(Return(expected_config.flush_all_events));

  std::vector<std::unique_ptr<Source>> sources{};
  sources.reserve(1);
  sources.emplace_back(std::move(source));

  const auto get_env = [](const char* /*unused*/) { return nullptr; };
  const auto config =
      Config::FromSourcesOrDefault(std::move(sources), default_config, get_env);
  ASSERT_EQ(config, expected_config);
}

// NOLINTNEXTLINE
TEST(Config, NeverReadsSecondarySourceIfPrimaryContainsAllConfigurations) {
  const Config default_config{
      .trace_file_path = "/path/to/trace/",
      .compression_strategy = util::compression::Strategy::kNone,
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = false,
  };
  const Config expected_config{
      .trace_file_path = "/path/to/other/trace/",
      .compression_strategy = util::compression::Strategy::kSnappy,
      .session_id = 10,
      .thread_event_buffer_capacity = 20,
      .max_reserved_event_buffer_slice_capacity = 30,
      .max_dynamic_event_buffer_slice_capacity = 40,
      .reserved_event_pool_capacity = 50,
      .dynamic_event_pool_capacity = 60,
      .dynamic_event_slice_borrow_cas_attempts = 70,
      .event_buffer_retention_duration_nanoseconds = 80,
      .max_flush_buffer_to_file_attempts = 90,
      .flush_all_events = true,
  };
  ASSERT_NE(default_config, expected_config);

  auto source_a = std::make_unique<SourceMock>();
  EXPECT_CALL(*source_a, Read())
      .WillOnce(Return(std::vector<Source::ReadError>{}));
  EXPECT_CALL(*source_a, IsRead())
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*source_a, TraceFilePath())
      .WillRepeatedly(Return(expected_config.trace_file_path));
  EXPECT_CALL(*source_a, CompressionStrategy())
      .WillRepeatedly(Return(expected_config.compression_strategy));
  EXPECT_CALL(*source_a, SessionId())
      .WillRepeatedly(Return(expected_config.session_id));
  EXPECT_CALL(*source_a, ThreadEventBufferCapacity())
      .WillRepeatedly(Return(expected_config.thread_event_buffer_capacity));
  EXPECT_CALL(*source_a, MaxReservedEventBufferSliceCapacity())
      .WillRepeatedly(
          Return(expected_config.max_reserved_event_buffer_slice_capacity));
  EXPECT_CALL(*source_a, MaxDynamicEventBufferSliceCapacity())
      .WillRepeatedly(
          Return(expected_config.max_dynamic_event_buffer_slice_capacity));
  EXPECT_CALL(*source_a, ReservedEventPoolCapacity())
      .WillRepeatedly(Return(expected_config.reserved_event_pool_capacity));
  EXPECT_CALL(*source_a, DynamicEventPoolCapacity())
      .WillRepeatedly(Return(expected_config.dynamic_event_pool_capacity));
  EXPECT_CALL(*source_a, DynamicEventSliceBorrowCasAttempts())
      .WillRepeatedly(
          Return(expected_config.dynamic_event_slice_borrow_cas_attempts));
  EXPECT_CALL(*source_a, EventBufferRetentionDurationNanoseconds())
      .WillRepeatedly(
          Return(expected_config.event_buffer_retention_duration_nanoseconds));
  EXPECT_CALL(*source_a, MaxFlushBufferToFileAttempts())
      .WillRepeatedly(
          Return(expected_config.max_flush_buffer_to_file_attempts));
  EXPECT_CALL(*source_a, FlushAllEvents())
      .WillRepeatedly(Return(expected_config.flush_all_events));

  auto source_b = std::make_unique<SourceMock>();
  EXPECT_CALL(*source_b, Read()).Times(0);
  EXPECT_CALL(*source_b, IsRead()).Times(0);
  EXPECT_CALL(*source_b, TraceFilePath()).Times(0);
  EXPECT_CALL(*source_b, CompressionStrategy()).Times(0);
  EXPECT_CALL(*source_b, SessionId()).Times(0);
  EXPECT_CALL(*source_b, ThreadEventBufferCapacity()).Times(0);
  EXPECT_CALL(*source_b, MaxReservedEventBufferSliceCapacity()).Times(0);
  EXPECT_CALL(*source_b, MaxDynamicEventBufferSliceCapacity()).Times(0);
  EXPECT_CALL(*source_b, ReservedEventPoolCapacity()).Times(0);
  EXPECT_CALL(*source_b, DynamicEventPoolCapacity()).Times(0);
  EXPECT_CALL(*source_b, DynamicEventSliceBorrowCasAttempts()).Times(0);
  EXPECT_CALL(*source_b, EventBufferRetentionDurationNanoseconds()).Times(0);
  EXPECT_CALL(*source_b, MaxFlushBufferToFileAttempts()).Times(0);
  EXPECT_CALL(*source_b, FlushAllEvents()).Times(0);

  std::vector<std::unique_ptr<Source>> sources{};
  sources.reserve(2);
  sources.emplace_back(std::move(source_a));
  sources.emplace_back(std::move(source_b));

  const auto get_env = [](const char* /*unused*/) { return nullptr; };
  const auto config =
      Config::FromSourcesOrDefault(std::move(sources), default_config, get_env);
  ASSERT_EQ(config, expected_config);
}

// NOLINTNEXTLINE
TEST(Config, ReadsSecondarySourceIfPrimaryDoesNotContainConfiguration) {
  const Config default_config{
      .trace_file_path = "/path/to/trace/",
      .compression_strategy = util::compression::Strategy::kNone,
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = false,
  };
  const Config expected_config{
      .trace_file_path = "/path/to/other/trace/",
      .compression_strategy = util::compression::Strategy::kSnappy,
      .session_id = 10,
      .thread_event_buffer_capacity = 20,
      .max_reserved_event_buffer_slice_capacity = 30,
      .max_dynamic_event_buffer_slice_capacity = 40,
      .reserved_event_pool_capacity = 50,
      .dynamic_event_pool_capacity = 60,
      .dynamic_event_slice_borrow_cas_attempts = 70,
      .event_buffer_retention_duration_nanoseconds = 80,
      .max_flush_buffer_to_file_attempts = 90,
      .flush_all_events = true,
  };
  ASSERT_NE(default_config, expected_config);

  auto source_a = std::make_unique<SourceMock>();
  EXPECT_CALL(*source_a, Read())
      .WillOnce(Return(std::vector<Source::ReadError>{}));
  EXPECT_CALL(*source_a, IsRead())
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*source_a, TraceFilePath()).WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source_a, CompressionStrategy())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source_a, SessionId()).WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source_a, ThreadEventBufferCapacity())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source_a, MaxReservedEventBufferSliceCapacity())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source_a, MaxDynamicEventBufferSliceCapacity())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source_a, ReservedEventPoolCapacity())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source_a, DynamicEventPoolCapacity())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source_a, DynamicEventSliceBorrowCasAttempts())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source_a, EventBufferRetentionDurationNanoseconds())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source_a, MaxFlushBufferToFileAttempts())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source_a, FlushAllEvents()).WillRepeatedly(Return(std::nullopt));

  auto source_b = std::make_unique<SourceMock>();
  EXPECT_CALL(*source_b, Read())
      .WillOnce(Return(std::vector<Source::ReadError>{}));
  EXPECT_CALL(*source_b, IsRead())
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*source_b, TraceFilePath())
      .WillRepeatedly(Return(expected_config.trace_file_path));
  EXPECT_CALL(*source_b, CompressionStrategy())
      .WillRepeatedly(Return(expected_config.compression_strategy));
  EXPECT_CALL(*source_b, SessionId())
      .WillRepeatedly(Return(expected_config.session_id));
  EXPECT_CALL(*source_b, ThreadEventBufferCapacity())
      .WillRepeatedly(Return(expected_config.thread_event_buffer_capacity));
  EXPECT_CALL(*source_b, MaxReservedEventBufferSliceCapacity())
      .WillRepeatedly(
          Return(expected_config.max_reserved_event_buffer_slice_capacity));
  EXPECT_CALL(*source_b, MaxDynamicEventBufferSliceCapacity())
      .WillRepeatedly(
          Return(expected_config.max_dynamic_event_buffer_slice_capacity));
  EXPECT_CALL(*source_b, ReservedEventPoolCapacity())
      .WillRepeatedly(Return(expected_config.reserved_event_pool_capacity));
  EXPECT_CALL(*source_b, DynamicEventPoolCapacity())
      .WillRepeatedly(Return(expected_config.dynamic_event_pool_capacity));
  EXPECT_CALL(*source_b, DynamicEventSliceBorrowCasAttempts())
      .WillRepeatedly(
          Return(expected_config.dynamic_event_slice_borrow_cas_attempts));
  EXPECT_CALL(*source_b, EventBufferRetentionDurationNanoseconds())
      .WillRepeatedly(
          Return(expected_config.event_buffer_retention_duration_nanoseconds));
  EXPECT_CALL(*source_b, MaxFlushBufferToFileAttempts())
      .WillRepeatedly(
          Return(expected_config.max_flush_buffer_to_file_attempts));
  EXPECT_CALL(*source_b, FlushAllEvents())
      .WillRepeatedly(Return(expected_config.flush_all_events));

  std::vector<std::unique_ptr<Source>> sources{};
  sources.reserve(2);
  sources.emplace_back(std::move(source_a));
  sources.emplace_back(std::move(source_b));

  const auto get_env = [](const char* /*unused*/) { return nullptr; };
  const auto config =
      Config::FromSourcesOrDefault(std::move(sources), default_config, get_env);
  ASSERT_EQ(config, expected_config);
}

TEST(Config, ExpandsEnvironmentVariables) {  // NOLINT
  const auto get_env = [](const char* key) -> const char* {
    if (key == nullptr) return nullptr;
    if (std::strncmp(key, util::env::kHomeKey.data(),
                     util::env::kHomeKey.size()) == 0) {
      return "/usr/you";
    }
    return nullptr;
  };
  const Config default_config{
      .trace_file_path = "~/path/to/trace/",
      .compression_strategy = util::compression::Strategy::kNone,
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = false,
  };
  const Config expected_config{
      .trace_file_path = "/usr/you/path/to/trace/",
      .compression_strategy = util::compression::Strategy::kNone,
      .session_id = 1,
      .thread_event_buffer_capacity = 2,
      .max_reserved_event_buffer_slice_capacity = 3,
      .max_dynamic_event_buffer_slice_capacity = 4,
      .reserved_event_pool_capacity = 5,
      .dynamic_event_pool_capacity = 6,
      .dynamic_event_slice_borrow_cas_attempts = 7,
      .event_buffer_retention_duration_nanoseconds = 8,
      .max_flush_buffer_to_file_attempts = 9,
      .flush_all_events = false,
  };
  ASSERT_NE(default_config, expected_config);

  const auto config_a =
      Config::FromSourcesOrDefault({}, default_config, get_env);
  ASSERT_EQ(config_a, expected_config);

  auto source = std::make_unique<SourceMock>();
  EXPECT_CALL(*source, Read())
      .WillOnce(Return(std::vector<Source::ReadError>{}));
  EXPECT_CALL(*source, IsRead())
      .WillOnce(Return(false))
      .WillRepeatedly(Return(true));
  EXPECT_CALL(*source, TraceFilePath())
      .WillRepeatedly(Return("$HOME/path/to/trace/"));
  EXPECT_CALL(*source, CompressionStrategy())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source, SessionId()).WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source, ThreadEventBufferCapacity())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source, MaxReservedEventBufferSliceCapacity())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source, MaxDynamicEventBufferSliceCapacity())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source, ReservedEventPoolCapacity())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source, DynamicEventPoolCapacity())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source, DynamicEventSliceBorrowCasAttempts())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source, EventBufferRetentionDurationNanoseconds())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source, MaxFlushBufferToFileAttempts())
      .WillRepeatedly(Return(std::nullopt));
  EXPECT_CALL(*source, FlushAllEvents()).WillRepeatedly(Return(std::nullopt));
  std::vector<std::unique_ptr<Source>> sources{};
  sources.reserve(1);
  sources.emplace_back(std::move(source));

  const auto config_b =
      Config::FromSourcesOrDefault(std::move(sources), default_config, get_env);
  ASSERT_EQ(config_b, expected_config);
}

}  // namespace
