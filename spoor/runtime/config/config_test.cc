#include "spoor/runtime/config/config.h"

#include <limits>
#include <string>
#include <unordered_map>

#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_buffer.h"

namespace {

using spoor::runtime::config::Config;
using SizeType = spoor::runtime::buffer::CircularBuffer<
    spoor::runtime::trace::Event>::SizeType;

TEST(Config, GetsUserProvidedValue) {  // NOLINT
  const auto get_env = [](const char* key) {
    using namespace spoor::runtime::config;
    const std::unordered_map<std::string_view, std::string_view> environment{
        {kTraceFilePathKey, "/path/to/file.extension"},
        {kSessionIdKey, "42"},
        {kThreadEventBufferCapacityKey, "42"},
        {kMaxReservedEventBufferSliceCapacityKey, "42"},
        {kMaxDynamicEventBufferSliceCapacityKey, "42"},
        {kReservedEventPoolCapacityKey, "42"},
        {kDynamicEventPoolCapacityKey, "42"},
        {kDynamicEventSliceBorrowCasAttemptsKey, "42"},
        {kEventBufferRetentionDurationNanosecondsKey, "42"},
        {kMaxFlushBufferToFileAttemptsKey, "42"},
        {kFlushAllEventsKey, "false"}};
    return environment.at(key).data();
  };
  const Config expected_options{
      .trace_file_path = "/path/to/file.extension",
      .session_id = 42,
      .thread_event_buffer_capacity = 42,
      .max_reserved_event_buffer_slice_capacity = 42,
      .max_dynamic_event_buffer_slice_capacity = 42,
      .reserved_event_pool_capacity = 42,
      .dynamic_event_pool_capacity = 42,
      .dynamic_event_slice_borrow_cas_attempts = 42,
      .event_buffer_retention_duration_nanoseconds = 42,
      .max_flush_buffer_to_file_attempts = 42,
      .flush_all_events = false};
  ASSERT_EQ(Config::FromEnv(get_env), expected_options);
}

TEST(Config, UsesDefaultValueWhenNotSpecified) {  // NOLINT
  const auto get_env = [](const char * /*unused*/) -> const char* {
    return nullptr;
  };
  Config expected_options{
      .trace_file_path = "",
      .session_id = 0,  // Ignored
      .thread_event_buffer_capacity = 10'000,
      .max_reserved_event_buffer_slice_capacity = 1'000,
      .max_dynamic_event_buffer_slice_capacity = 1'000,
      .reserved_event_pool_capacity = 0,
      .dynamic_event_pool_capacity = std::numeric_limits<SizeType>::max(),
      .dynamic_event_slice_borrow_cas_attempts = 1,
      .event_buffer_retention_duration_nanoseconds = 0,
      .max_flush_buffer_to_file_attempts = std::numeric_limits<int32>::max(),
      .flush_all_events = true};
  const auto options = Config::FromEnv(get_env);
  // Ignore `session_id` which is randomly generated.
  expected_options.session_id = options.session_id;
  ASSERT_EQ(options, expected_options);
}

}  // namespace
