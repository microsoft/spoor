// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#include <limits>

#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "gmock/gmock.h"
#include "gsl/gsl"
#include "gtest/gtest.h"

namespace {

using testing::MockFunction;

constexpr absl::Duration kNotificationTimeout{absl::Milliseconds(1'000)};

ACTION_P(Notify, notification) {  // NOLINT
  notification->Notify();
}

TEST(Runtime, Initialize) {  // NOLINT
  for (auto iteration{0}; iteration < 3; ++iteration) {
    ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
    ASSERT_FALSE(_spoor_runtime_RuntimeInitialized());
    _spoor_runtime_InitializeRuntime();
    ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
    ASSERT_TRUE(_spoor_runtime_RuntimeInitialized());
    _spoor_runtime_InitializeRuntime();
    ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
    ASSERT_TRUE(_spoor_runtime_RuntimeInitialized());
    _spoor_runtime_DeinitializeRuntime();
    ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
    ASSERT_FALSE(_spoor_runtime_RuntimeInitialized());
    _spoor_runtime_DeinitializeRuntime();
    ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
    ASSERT_FALSE(_spoor_runtime_RuntimeInitialized());
  }
}

TEST(Runtime, Enable) {  // NOLINT
  ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
  _spoor_runtime_InitializeRuntime();
  ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
  for (auto iteration{0}; iteration < 3; ++iteration) {
    _spoor_runtime_EnableRuntime();
    ASSERT_TRUE(_spoor_runtime_RuntimeEnabled());
    _spoor_runtime_EnableRuntime();
    ASSERT_TRUE(_spoor_runtime_RuntimeEnabled());
    _spoor_runtime_DisableRuntime();
    ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
    _spoor_runtime_DisableRuntime();
    ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
  }
  _spoor_runtime_EnableRuntime();
  ASSERT_TRUE(_spoor_runtime_RuntimeEnabled());
  _spoor_runtime_DeinitializeRuntime();
  ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
}

TEST(Runtime, FlushTraceEvents) {  // NOLINT
  _spoor_runtime_FlushTraceEvents({});
}

TEST(Runtime, ClearTraceEvents) {  // NOLINT
  _spoor_runtime_DeinitializeRuntime();
  _spoor_runtime_ClearTraceEvents();
  _spoor_runtime_InitializeRuntime();
  _spoor_runtime_ClearTraceEvents();
  _spoor_runtime_DeinitializeRuntime();
}

namespace flused_trace_files_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(_spoor_runtime_TraceFiles)> callback_{};

TEST(Runtime, FlushedTraceFiles) {  // NOLINT
  _spoor_runtime_FlushedTraceFiles({});

  constexpr _spoor_runtime_TraceFiles expected_trace_files{
      .file_paths_size = 0, .file_path_sizes = nullptr, .file_paths = nullptr};
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

}  // namespace flused_trace_files_test

namespace delete_flushed_trace_files_older_than_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(_spoor_runtime_DeletedFilesInfo)> callback_{};

TEST(Runtime, DeleteFlushedTraceFilesOlderThan) {  // NOLINT
  _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
      std::numeric_limits<_spoor_runtime_SystemTimestampSeconds>::max(), {});

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

}  // namespace delete_flushed_trace_files_older_than_test

TEST(Runtime, GetConfig) {  // NOLINT
  const auto config = _spoor_runtime_GetConfig();
  ASSERT_NE(config.trace_file_path, nullptr);
}

TEST(Runtime, StubImplementation) {  // NOLINT
  ASSERT_FALSE(_spoor_runtime_StubImplementation());
}

}  // namespace
