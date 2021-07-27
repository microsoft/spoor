// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#include <filesystem>
#include <functional>
#include <limits>
#include <vector>

#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gsl/gsl"
#include "gtest/gtest.h"

namespace {

using testing::MockFunction;
using testing::Test;

constexpr absl::Duration kNotificationTimeout{absl::Milliseconds(1'000)};

class RuntimeAutoInitialize : public Test {
 protected:
  auto SetUp() -> void override { spoor::runtime::Initialize(); }
  auto TearDown() -> void override { spoor::runtime::Deinitialize(); }
};

ACTION_P(Notify, notification) {  // NOLINT
  notification->Notify();
}

TEST(Runtime, Initialize) {  // NOLINT
  for (auto iteration{0}; iteration < 3; ++iteration) {
    ASSERT_FALSE(_spoor_runtime_Enabled());
    ASSERT_FALSE(_spoor_runtime_Initialized());
    _spoor_runtime_Initialize();
    ASSERT_FALSE(_spoor_runtime_Enabled());
    ASSERT_TRUE(_spoor_runtime_Initialized());
    _spoor_runtime_Initialize();
    ASSERT_FALSE(_spoor_runtime_Enabled());
    ASSERT_TRUE(_spoor_runtime_Initialized());
    _spoor_runtime_Deinitialize();
    ASSERT_FALSE(_spoor_runtime_Enabled());
    ASSERT_FALSE(_spoor_runtime_Initialized());
    _spoor_runtime_Deinitialize();
    ASSERT_FALSE(_spoor_runtime_Enabled());
    ASSERT_FALSE(_spoor_runtime_Initialized());

    ASSERT_FALSE(spoor::runtime::Enabled());
    ASSERT_FALSE(spoor::runtime::Initialized());
    spoor::runtime::Initialize();
    ASSERT_FALSE(spoor::runtime::Enabled());
    ASSERT_TRUE(spoor::runtime::Initialized());
    spoor::runtime::Initialize();
    ASSERT_FALSE(spoor::runtime::Enabled());
    ASSERT_TRUE(spoor::runtime::Initialized());
    spoor::runtime::Deinitialize();
    ASSERT_FALSE(spoor::runtime::Enabled());
    ASSERT_FALSE(spoor::runtime::Initialized());
    spoor::runtime::Deinitialize();
    ASSERT_FALSE(spoor::runtime::Enabled());
    ASSERT_FALSE(spoor::runtime::Initialized());
  }
}

TEST(Runtime, Enable) {  // NOLINT
  ASSERT_FALSE(_spoor_runtime_Enabled());
  _spoor_runtime_Initialize();
  ASSERT_FALSE(_spoor_runtime_Enabled());
  for (auto iteration{0}; iteration < 3; ++iteration) {
    _spoor_runtime_Enable();
    ASSERT_TRUE(_spoor_runtime_Enabled());
    _spoor_runtime_Enable();
    ASSERT_TRUE(_spoor_runtime_Enabled());
    _spoor_runtime_Disable();
    ASSERT_FALSE(_spoor_runtime_Enabled());
    _spoor_runtime_Disable();
    ASSERT_FALSE(_spoor_runtime_Enabled());
  }
  _spoor_runtime_Enable();
  ASSERT_TRUE(_spoor_runtime_Enabled());
  _spoor_runtime_Deinitialize();
  ASSERT_FALSE(_spoor_runtime_Enabled());

  ASSERT_FALSE(spoor::runtime::Enabled());
  spoor::runtime::Initialize();
  ASSERT_FALSE(spoor::runtime::Enabled());
  for (auto iteration{0}; iteration < 3; ++iteration) {
    spoor::runtime::Enable();
    ASSERT_TRUE(spoor::runtime::Enabled());
    spoor::runtime::Enable();
    ASSERT_TRUE(spoor::runtime::Enabled());
    spoor::runtime::Disable();
    ASSERT_FALSE(spoor::runtime::Enabled());
    spoor::runtime::Disable();
    ASSERT_FALSE(spoor::runtime::Enabled());
  }
  spoor::runtime::Enable();
  ASSERT_TRUE(spoor::runtime::Enabled());
  spoor::runtime::Deinitialize();
  ASSERT_FALSE(spoor::runtime::Enabled());
}

namespace flush_trace_events_running_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(void)> callback_{};

TEST_F(RuntimeAutoInitialize, FlushTraceEventsRunningCApi) {  // NOLINT
  _spoor_runtime_FlushTraceEvents({});

  MockFunction<void(void)> callback{};
  absl::Notification done{};
  callback_ = callback.AsStdFunction();
  EXPECT_CALL(callback, Call()).WillOnce(Notify(&done));
  _spoor_runtime_FlushTraceEvents([] { callback_(); });
  const auto success =
      done.WaitForNotificationWithTimeout(kNotificationTimeout);
  ASSERT_TRUE(success);
}

}  // namespace flush_trace_events_running_test

TEST_F(RuntimeAutoInitialize, FlushTraceEventsRunningCcApi) {  // NOLINT
  spoor::runtime::FlushTraceEvents({});

  MockFunction<void()> callback{};
  absl::Notification done{};
  EXPECT_CALL(callback, Call()).WillOnce(Notify(&done));
  spoor::runtime::FlushTraceEvents(callback.AsStdFunction());
  const auto success =
      done.WaitForNotificationWithTimeout(kNotificationTimeout);
  ASSERT_TRUE(success);
}

namespace flush_trace_events_not_running_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(void)> callback_{};

TEST(Runtime, FlushTraceEventsNotRunning) {  // NOLINT
  {
    _spoor_runtime_FlushTraceEvents({});

    MockFunction<void(void)> callback{};
    absl::Notification done{};
    callback_ = callback.AsStdFunction();
    EXPECT_CALL(callback, Call()).WillOnce(Notify(&done));
    _spoor_runtime_FlushTraceEvents([] { callback_(); });
    const auto success =
        done.WaitForNotificationWithTimeout(kNotificationTimeout);
    ASSERT_TRUE(success);
  }
  {
    spoor::runtime::FlushTraceEvents({});

    MockFunction<void()> callback{};
    absl::Notification done{};
    EXPECT_CALL(callback, Call()).WillOnce(Notify(&done));
    spoor::runtime::FlushTraceEvents(callback.AsStdFunction());
    const auto success =
        done.WaitForNotificationWithTimeout(kNotificationTimeout);
    ASSERT_TRUE(success);
  }
}

}  // namespace flush_trace_events_not_running_test

TEST(Runtime, ClearTraceEvents) {  // NOLINT
  _spoor_runtime_Deinitialize();
  _spoor_runtime_ClearTraceEvents();
  _spoor_runtime_Initialize();
  _spoor_runtime_ClearTraceEvents();
  _spoor_runtime_Deinitialize();

  spoor::runtime::Deinitialize();
  spoor::runtime::ClearTraceEvents();
  spoor::runtime::Initialize();
  spoor::runtime::ClearTraceEvents();
  spoor::runtime::Deinitialize();
}

namespace flushed_trace_files_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(_spoor_runtime_TraceFiles)> callback_{};

TEST(Runtime, FlushedTraceFiles) {  // NOLINT
  {
    constexpr _spoor_runtime_TraceFiles expected_trace_files{
        .file_paths_size = 0,
        .file_path_sizes = nullptr,
        .file_paths = nullptr};
    MockFunction<void(_spoor_runtime_TraceFiles)> callback{};
    absl::Notification done{};
    callback_ = callback.AsStdFunction();
    EXPECT_CALL(callback, Call(expected_trace_files)).WillOnce(Notify(&done));
    _spoor_runtime_FlushedTraceFiles(
        [](auto trace_files) { callback_(trace_files); });
    const auto success =
        done.WaitForNotificationWithTimeout(kNotificationTimeout);
    ASSERT_TRUE(success);
  }
  {
    const std::vector<std::filesystem::path> expected_trace_files{};
    MockFunction<void(std::vector<std::filesystem::path>)> callback{};
    absl::Notification done{};
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
  {
    constexpr _spoor_runtime_DeletedFilesInfo expected_deleted_files_info{
        .deleted_files = 0, .deleted_bytes = 0};
    MockFunction<void(_spoor_runtime_DeletedFilesInfo)> callback{};
    absl::Notification done{};
    callback_ = callback.AsStdFunction();
    EXPECT_CALL(callback, Call(expected_deleted_files_info))
        .WillOnce(Notify(&done));
    _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
        std::numeric_limits<_spoor_runtime_SystemTimestampSeconds>::max(),
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
        std::numeric_limits<spoor::runtime::SystemTimestampSeconds>::max(),
        callback.AsStdFunction());
    const auto success =
        done.WaitForNotificationWithTimeout(kNotificationTimeout);
    ASSERT_TRUE(success);
  }
}

}  // namespace delete_flushed_trace_files_older_than_test

TEST(Runtime, GetConfig) {  // NOLINT
  {
    const _spoor_runtime_Config expected_config{};
    const auto config = _spoor_runtime_GetConfig();
    ASSERT_NE(config, expected_config);
  }
  {
    const spoor::runtime::Config expected_config{};
    const auto config = spoor::runtime::GetConfig();
    ASSERT_NE(config, expected_config);
  }
}

TEST(Runtime, StubImplementation) {  // NOLINT
  ASSERT_FALSE(_spoor_runtime_StubImplementation());
  ASSERT_FALSE(spoor::runtime::StubImplementation());
}

}  // namespace
