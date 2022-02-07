// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <limits>

#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "spoor/runtime/runtime.h"

namespace {

using testing::MockFunction;

constexpr absl::Duration kNotificationTimeout{absl::Milliseconds(1'000)};

ACTION_P(Notify, notification) {  // NOLINT
  notification->Notify();
}

TEST(Runtime, Initialize) {  // NOLINT
  ASSERT_FALSE(spoor::runtime::Initialized());
  spoor::runtime::Initialize();
  ASSERT_FALSE(spoor::runtime::Initialized());
  spoor::runtime::Deinitialize();
  ASSERT_FALSE(spoor::runtime::Initialized());
}

TEST(Runtime, Enable) {  // NOLINT
  ASSERT_FALSE(spoor::runtime::Enabled());
  spoor::runtime::Enable();
  ASSERT_FALSE(spoor::runtime::Enabled());
  spoor::runtime::Disable();
  ASSERT_FALSE(spoor::runtime::Enabled());
}

TEST(Runtime, LogEvent) {  // NOLINT
  spoor::runtime::LogEvent(1, 2, 3, 4);
  spoor::runtime::LogEvent(1, 2, 3);
}

TEST(Runtime, FlushTraceEvents) {  // NOLINT
  spoor::runtime::FlushTraceEvents({});

  MockFunction<void()> callback{};
  absl::Notification done{};
  EXPECT_CALL(callback, Call()).WillOnce(Notify(&done));
  spoor::runtime::FlushTraceEvents(callback.AsStdFunction());
  const auto success =
      done.WaitForNotificationWithTimeout(kNotificationTimeout);
  ASSERT_TRUE(success);
}

TEST(Runtime, ClearTraceEvents) {  // NOLINT
  spoor::runtime::ClearTraceEvents();
}

TEST(Runtime, FlushedTraceFiles) {  // NOLINT
  spoor::runtime::FlushedTraceFiles({});

  MockFunction<void(std::vector<std::filesystem::path>)> callback{};
  absl::Notification done{};
  const std::vector<std::filesystem::path> expected_trace_files{};
  EXPECT_CALL(callback, Call(expected_trace_files)).WillOnce(Notify(&done));
  spoor::runtime::FlushedTraceFiles(callback.AsStdFunction());
  const auto success =
      done.WaitForNotificationWithTimeout(kNotificationTimeout);
  ASSERT_TRUE(success);
}

TEST(Runtime, DeleteFlushedTraceFilesOlderThan) {  // NOLINT
  spoor::runtime::DeleteFlushedTraceFilesOlderThan(
      std::numeric_limits<spoor::runtime::SystemTimestampSeconds>::min(), {});

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

TEST(Runtime, GetConfig) {  // NOLINT
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

TEST(Runtime, StubImplementation) {  // NOLINT
  ASSERT_TRUE(spoor::runtime::StubImplementation());
}

}  // namespace
