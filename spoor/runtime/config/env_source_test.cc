// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/config/env_source.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>

#include "absl/strings/str_format.h"
#include "gtest/gtest.h"
#include "util/compression/compressor.h"
#include "util/flat_map/flat_map.h"

namespace {

using spoor::runtime::config::EnvSource;
using spoor::runtime::config::kCompressionStrategyEnvKey;
using spoor::runtime::config::kDynamicEventPoolCapacityEnvKey;
using spoor::runtime::config::kDynamicEventSliceBorrowCasAttemptsEnvKey;
using spoor::runtime::config::kEnvConfigKeys;
using spoor::runtime::config::kEventBufferRetentionDurationNanosecondsEnvKey;
using spoor::runtime::config::kFlushAllEventsEnvKey;
using spoor::runtime::config::kMaxDynamicEventBufferSliceCapacityEnvKey;
using spoor::runtime::config::kMaxFlushBufferToFileAttemptsEnvKey;
using spoor::runtime::config::kMaxReservedEventBufferSliceCapacityEnvKey;
using spoor::runtime::config::kReservedEventPoolCapacityEnvKey;
using spoor::runtime::config::kSessionIdEnvKey;
using spoor::runtime::config::kThreadEventBufferCapacityEnvKey;
using spoor::runtime::config::kTraceFilePathEnvKey;

TEST(EnvSource, ReadsConfig) {  // NOLINT
  const auto get_env = [](const char* key) {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view,
                                      kEnvConfigKeys.size()>
        environment{
            {kTraceFilePathEnvKey, "/path/to/trace/"},
            {kCompressionStrategyEnvKey, "snappy"},
            {kSessionIdEnvKey, "1"},
            {kThreadEventBufferCapacityEnvKey, "2"},
            {kMaxReservedEventBufferSliceCapacityEnvKey, "3"},
            {kMaxDynamicEventBufferSliceCapacityEnvKey, "4"},
            {kReservedEventPoolCapacityEnvKey, "5"},
            {kDynamicEventPoolCapacityEnvKey, "6"},
            {kDynamicEventSliceBorrowCasAttemptsEnvKey, "7"},
            {kEventBufferRetentionDurationNanosecondsEnvKey, "8"},
            {kMaxFlushBufferToFileAttemptsEnvKey, "9"},
            {kFlushAllEventsEnvKey, "true"},
        };
    return environment.FirstValueForKey(key).value().data();
  };
  EnvSource env_config{{.get_env = get_env}};

  ASSERT_FALSE(env_config.TraceFilePath().has_value());
  ASSERT_FALSE(env_config.CompressionStrategy().has_value());
  ASSERT_FALSE(env_config.SessionId().has_value());
  ASSERT_FALSE(env_config.ThreadEventBufferCapacity().has_value());
  ASSERT_FALSE(env_config.MaxReservedEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(env_config.MaxDynamicEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(env_config.ReservedEventPoolCapacity().has_value());
  ASSERT_FALSE(env_config.DynamicEventPoolCapacity().has_value());
  ASSERT_FALSE(env_config.DynamicEventSliceBorrowCasAttempts().has_value());
  ASSERT_FALSE(
      env_config.EventBufferRetentionDurationNanoseconds().has_value());
  ASSERT_FALSE(env_config.MaxFlushBufferToFileAttempts().has_value());
  ASSERT_FALSE(env_config.FlushAllEvents().has_value());

  ASSERT_FALSE(env_config.IsRead());
  const auto read_errors = env_config.Read();
  ASSERT_TRUE(env_config.IsRead());
  ASSERT_TRUE(read_errors.empty());

  const auto trace_file_path = env_config.TraceFilePath();
  ASSERT_TRUE(trace_file_path.has_value());
  ASSERT_EQ(trace_file_path.value(), "/path/to/trace/");
  const auto compression_strategy = env_config.CompressionStrategy();
  ASSERT_TRUE(compression_strategy.has_value());
  ASSERT_EQ(compression_strategy.value(), util::compression::Strategy::kSnappy);
  const auto session_id = env_config.SessionId();
  ASSERT_TRUE(session_id.has_value());
  ASSERT_EQ(session_id.value(), 1);
  const auto thread_event_buffer_capacity =
      env_config.ThreadEventBufferCapacity();
  ASSERT_TRUE(thread_event_buffer_capacity.has_value());
  ASSERT_EQ(thread_event_buffer_capacity.value(), 2);
  const auto max_reserved_event_buffer_slice_capacity =
      env_config.MaxReservedEventBufferSliceCapacity();
  ASSERT_TRUE(max_reserved_event_buffer_slice_capacity.has_value());
  ASSERT_EQ(max_reserved_event_buffer_slice_capacity.value(), 3);
  const auto max_dynamic_event_buffer_slice_capacity =
      env_config.MaxDynamicEventBufferSliceCapacity();
  ASSERT_TRUE(max_dynamic_event_buffer_slice_capacity.has_value());
  ASSERT_EQ(max_dynamic_event_buffer_slice_capacity.value(), 4);
  const auto reserved_event_pool_capacity =
      env_config.ReservedEventPoolCapacity();
  ASSERT_TRUE(reserved_event_pool_capacity.has_value());
  ASSERT_EQ(reserved_event_pool_capacity.value(), 5);
  const auto dynamic_event_pool_capacity =
      env_config.DynamicEventPoolCapacity();
  ASSERT_TRUE(dynamic_event_pool_capacity.has_value());
  ASSERT_EQ(dynamic_event_pool_capacity.value(), 6);
  const auto dynamic_event_slice_borrow_cas_attempts =
      env_config.DynamicEventSliceBorrowCasAttempts();
  ASSERT_TRUE(dynamic_event_slice_borrow_cas_attempts.has_value());
  ASSERT_EQ(dynamic_event_slice_borrow_cas_attempts.value(), 7);
  const auto event_buffer_retention_duration_nanoseconds =
      env_config.EventBufferRetentionDurationNanoseconds();
  ASSERT_TRUE(event_buffer_retention_duration_nanoseconds.has_value());
  ASSERT_EQ(event_buffer_retention_duration_nanoseconds, 8);
  const auto max_flush_buffer_to_file_attempts =
      env_config.MaxFlushBufferToFileAttempts();
  ASSERT_TRUE(max_flush_buffer_to_file_attempts.has_value());
  ASSERT_EQ(max_flush_buffer_to_file_attempts.value(), 9);
  const auto flush_all_events = env_config.FlushAllEvents();
  ASSERT_TRUE(flush_all_events.has_value());
  ASSERT_TRUE(flush_all_events.value());
}

TEST(EnvSource, NormalizesCompressionStrategy) {  // NOLINT
  const auto get_env = [](const char* key) -> const char* {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 1>
        environment{{kCompressionStrategyEnvKey, "     sNaPpY    "}};
    auto value = environment.FirstValueForKey(key);
    if (value.has_value()) return value.value().data();
    return nullptr;
  };
  EnvSource env_config{{.get_env = get_env}};

  const auto read_errors = env_config.Read();
  ASSERT_TRUE(read_errors.empty());

  const auto compression_strategy = env_config.CompressionStrategy();
  ASSERT_TRUE(compression_strategy.has_value());
  ASSERT_EQ(compression_strategy.value(), util::compression::Strategy::kSnappy);
}

TEST(FileConfig, HandlesEmptyEnv) {  // NOLINT
  const auto get_env = [](const char* /*unused*/) { return nullptr; };
  EnvSource env_config{{.get_env = get_env}};

  const auto read_errors = env_config.Read();
  ASSERT_TRUE(read_errors.empty());

  ASSERT_FALSE(env_config.TraceFilePath().has_value());
  ASSERT_FALSE(env_config.CompressionStrategy().has_value());
  ASSERT_FALSE(env_config.SessionId().has_value());
  ASSERT_FALSE(env_config.ThreadEventBufferCapacity().has_value());
  ASSERT_FALSE(env_config.MaxReservedEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(env_config.MaxDynamicEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(env_config.ReservedEventPoolCapacity().has_value());
  ASSERT_FALSE(env_config.DynamicEventPoolCapacity().has_value());
  ASSERT_FALSE(env_config.DynamicEventSliceBorrowCasAttempts().has_value());
  ASSERT_FALSE(
      env_config.EventBufferRetentionDurationNanoseconds().has_value());
  ASSERT_FALSE(env_config.MaxFlushBufferToFileAttempts().has_value());
  ASSERT_FALSE(env_config.FlushAllEvents().has_value());
}

TEST(EnvSource, HandlesBadValue) {  // NOLINT
  const auto get_env = [](const char* key) {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view,
                                      kEnvConfigKeys.size()>
        environment{
            {kTraceFilePathEnvKey, "42"},
            {kCompressionStrategyEnvKey, "42"},
            {kSessionIdEnvKey, "foo"},
            {kThreadEventBufferCapacityEnvKey, "foo"},
            {kMaxReservedEventBufferSliceCapacityEnvKey, "foo"},
            {kMaxDynamicEventBufferSliceCapacityEnvKey, "foo"},
            {kReservedEventPoolCapacityEnvKey, "foo"},
            {kDynamicEventPoolCapacityEnvKey, "foo"},
            {kDynamicEventSliceBorrowCasAttemptsEnvKey, "foo"},
            {kEventBufferRetentionDurationNanosecondsEnvKey, "foo"},
            {kMaxFlushBufferToFileAttemptsEnvKey, "foo"},
            {kFlushAllEventsEnvKey, "foo"},
        };
    return environment.FirstValueForKey(key).value().data();
  };
  EnvSource env_config{{.get_env = get_env}};

  ASSERT_FALSE(env_config.IsRead());
  const auto read_errors = env_config.Read();
  ASSERT_TRUE(env_config.IsRead());

  // Reading the trace file path into a string cannot fail.
  ASSERT_EQ(read_errors.size(), kEnvConfigKeys.size() - 1);
  const auto error_messages = [&read_errors] {
    std::vector<std::string> messages{};
    std::transform(std::cbegin(read_errors), std::cend(read_errors),
                   std::back_inserter(messages),
                   [](const auto& error) { return error.message; });
    std::sort(std::begin(messages), std::end(messages));
    return messages;
  }();
  const auto expected_error_messages = [&] {
    std::vector<std::string_view> keys{};
    keys.reserve(std::size(kEnvConfigKeys) - 1);
    std::copy_if(std::cbegin(kEnvConfigKeys), std::cend(kEnvConfigKeys),
                 std::back_inserter(keys),
                 [](const auto key) { return key != kTraceFilePathEnvKey; });
    std::vector<std::string> messages{};
    std::transform(std::cbegin(keys), std::cend(keys),
                   std::back_inserter(messages), [](const auto key) {
                     return absl::StrFormat(
                         "Cannot parse value for key \"%s\".", key);
                   });
    std::sort(std::begin(messages), std::end(messages));
    return messages;
  }();
  ASSERT_EQ(error_messages, expected_error_messages);

  ASSERT_TRUE(env_config.TraceFilePath().has_value());
  ASSERT_FALSE(env_config.CompressionStrategy().has_value());
  ASSERT_FALSE(env_config.SessionId().has_value());
  ASSERT_FALSE(env_config.ThreadEventBufferCapacity().has_value());
  ASSERT_FALSE(env_config.MaxReservedEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(env_config.MaxDynamicEventBufferSliceCapacity().has_value());
  ASSERT_FALSE(env_config.ReservedEventPoolCapacity().has_value());
  ASSERT_FALSE(env_config.DynamicEventPoolCapacity().has_value());
  ASSERT_FALSE(env_config.DynamicEventSliceBorrowCasAttempts().has_value());
  ASSERT_FALSE(
      env_config.EventBufferRetentionDurationNanoseconds().has_value());
  ASSERT_FALSE(env_config.MaxFlushBufferToFileAttempts().has_value());
  ASSERT_FALSE(env_config.FlushAllEvents().has_value());
}

}  // namespace
