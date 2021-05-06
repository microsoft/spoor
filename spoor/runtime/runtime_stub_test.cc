// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <limits>

#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/runtime/runtime.h"

namespace {

using testing::MockFunction;

constexpr absl::Duration kNotificationTimeout{absl::Milliseconds(1'000)};

ACTION_P(Notify, notification) {  // NOLINT
  notification->Notify();
}

TEST(Runtime, Initialize) {  // NOLINT
  ASSERT_FALSE(_spoor_runtime_Initialized());
  _spoor_runtime_Initialize();
  ASSERT_FALSE(_spoor_runtime_Initialized());
  _spoor_runtime_Deinitialize();
  ASSERT_FALSE(_spoor_runtime_Initialized());

  ASSERT_FALSE(spoor::runtime::Initialized());
  spoor::runtime::Initialize();
  ASSERT_FALSE(spoor::runtime::Initialized());
  spoor::runtime::Deinitialize();
  ASSERT_FALSE(spoor::runtime::Initialized());
}

TEST(Runtime, Enable) {  // NOLINT
  ASSERT_FALSE(_spoor_runtime_Enabled());
  _spoor_runtime_Enable();
  ASSERT_FALSE(_spoor_runtime_Enabled());
  _spoor_runtime_Disable();
  ASSERT_FALSE(_spoor_runtime_Enabled());

  ASSERT_FALSE(spoor::runtime::Enabled());
  spoor::runtime::Enable();
  ASSERT_FALSE(spoor::runtime::Enabled());
  spoor::runtime::Disable();
  ASSERT_FALSE(spoor::runtime::Enabled());
}

TEST(Runtime, LogEvent) {  // NOLINT
  _spoor_runtime_LogEventWithTimestamp(1, 2, 3, 4);
  _spoor_runtime_LogEvent(1, 2, 3);
  _spoor_runtime_LogFunctionEntry(1);
  _spoor_runtime_LogFunctionExit(1);

  spoor::runtime::LogEvent(1, 2, 3, 4);
  spoor::runtime::LogEvent(1, 2, 3);
}

namespace flush_trace_events_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void()> callback_{};

TEST(Runtime, FlushTraceEvents) {  // NOLINT
  _spoor_runtime_FlushTraceEvents({});
  spoor::runtime::FlushTraceEvents({});

  {
    MockFunction<void()> callback{};
    absl::Notification done{};
    callback_ = callback.AsStdFunction();
    EXPECT_CALL(callback, Call()).WillOnce(Notify(&done));
    _spoor_runtime_FlushTraceEvents([] { callback_(); });
    const auto success =
        done.WaitForNotificationWithTimeout(kNotificationTimeout);
    ASSERT_TRUE(success);
  }
  {
    MockFunction<void()> callback{};
    absl::Notification done{};
    EXPECT_CALL(callback, Call()).WillOnce(Notify(&done));
    spoor::runtime::FlushTraceEvents(callback.AsStdFunction());
    const auto success =
        done.WaitForNotificationWithTimeout(kNotificationTimeout);
    ASSERT_TRUE(success);
  }
}

}  // namespace flush_trace_events_test

TEST(Runtime, ClearTraceEvents) {  // NOLINT
  _spoor_runtime_ClearTraceEvents();

  spoor::runtime::ClearTraceEvents();
}

namespace flushed_trace_files_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(_spoor_runtime_TraceFiles)> callback_{};

TEST(Runtime, FlushedTraceFiles) {  // NOLINT
  _spoor_runtime_FlushedTraceFiles({});
  spoor::runtime::FlushedTraceFiles({});

  {
    MockFunction<void(_spoor_runtime_TraceFiles)> callback{};
    absl::Notification done{};
    constexpr _spoor_runtime_TraceFiles expected_trace_files{
        .file_paths_size = 0,
        .file_path_sizes = nullptr,
        .file_paths = nullptr};
    callback_ = callback.AsStdFunction();
    EXPECT_CALL(callback, Call(expected_trace_files)).WillOnce(Notify(&done));
    _spoor_runtime_FlushedTraceFiles(
        [](auto trace_files) { callback_(trace_files); });
    const auto success =
        done.WaitForNotificationWithTimeout(kNotificationTimeout);
    ASSERT_TRUE(success);
  }
  {
    MockFunction<void(std::vector<std::filesystem::path>)> callback{};
    absl::Notification done{};
    const std::vector<std::filesystem::path> expected_trace_files{};
    EXPECT_CALL(callback, Call(expected_trace_files)).WillOnce(Notify(&done));
    spoor::runtime::FlushedTraceFiles(callback.AsStdFunction());
    const auto success =
        done.WaitForNotificationWithTimeout(kNotificationTimeout);
    ASSERT_TRUE(success);
  }
}

}  // namespace flushed_trace_files_test

namespace delete_flushed_trace_files_older_than_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(_spoor_runtime_DeletedFilesInfo)> callback_{};

TEST(Runtime, DeleteFlushedTraceFilesOlderThan) {  // NOLINT
  _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
      std::numeric_limits<_spoor_runtime_SystemTimestampSeconds>::min(), {});
  spoor::runtime::DeleteFlushedTraceFilesOlderThan(
      std::numeric_limits<_spoor_runtime_SystemTimestampSeconds>::min(), {});

  {
    constexpr _spoor_runtime_DeletedFilesInfo expected_deleted_files_info{
        .deleted_files = 0, .deleted_bytes = 0};
    MockFunction<void(_spoor_runtime_DeletedFilesInfo)> callback{};
    absl::Notification done{};
    callback_ = callback.AsStdFunction();
    EXPECT_CALL(callback, Call(expected_deleted_files_info))
        .WillOnce(Notify(&done));
    _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
        std::numeric_limits<_spoor_runtime_SystemTimestampSeconds>::min(),
        [](auto deleted_files_info) { callback_(deleted_files_info); });
    const auto success =
        done.WaitForNotificationWithTimeout(kNotificationTimeout);
    ASSERT_TRUE(success);
  }
  {
    constexpr spoor::runtime::DeletedFilesInfo expected_deleted_files_info{
        .deleted_files = 0, .deleted_bytes = 0};
    MockFunction<void(spoor::runtime::DeletedFilesInfo)> callback{};
    absl::Notification done{};
    EXPECT_CALL(callback, Call(expected_deleted_files_info))
        .WillOnce(Notify(&done));
    spoor::runtime::DeleteFlushedTraceFilesOlderThan(
        std::numeric_limits<spoor::runtime::SystemTimestampSeconds>::min(),
        callback.AsStdFunction());
    const auto success =
        done.WaitForNotificationWithTimeout(kNotificationTimeout);
    ASSERT_TRUE(success);
  }
}

}  // namespace delete_flushed_trace_files_older_than_test

TEST(Runtime, GetConfig) {  // NOLINT
  {
    constexpr _spoor_runtime_Config expected_config{
        .trace_file_path_size = 0,
        .trace_file_path = nullptr,
        .session_id = 0,
        .thread_event_buffer_capacity = 0,
        .max_reserved_event_buffer_slice_capacity = 0,
        .max_dynamic_event_buffer_slice_capacity = 0,
        .reserved_event_pool_capacity = 0,
        .dynamic_event_pool_capacity = 0,
        .dynamic_event_slice_borrow_cas_attempts = 0,
        .event_buffer_retention_duration_nanoseconds = 0,
        .max_flush_buffer_to_file_attempts = 0,
        .flush_all_events = false};
    const auto config = _spoor_runtime_GetConfig();
    ASSERT_EQ(config, expected_config);
  }
  {
    const spoor::runtime::Config expected_config{
        .trace_file_path = {},
        .session_id = 0,
        .thread_event_buffer_capacity = 0,
        .max_reserved_event_buffer_slice_capacity = 0,
        .max_dynamic_event_buffer_slice_capacity = 0,
        .reserved_event_pool_capacity = 0,
        .dynamic_event_pool_capacity = 0,
        .dynamic_event_slice_borrow_cas_attempts = 0,
        .event_buffer_retention_duration_nanoseconds = 0,
        .max_flush_buffer_to_file_attempts = 0,
        .flush_all_events = false};
    const auto config = spoor::runtime::GetConfig();
    ASSERT_EQ(config, expected_config);
  }
}

TEST(Runtime, StubImplementation) {  // NOLINT
  ASSERT_TRUE(_spoor_runtime_StubImplementation());
  ASSERT_TRUE(spoor::runtime::StubImplementation());
}

}  // namespace
