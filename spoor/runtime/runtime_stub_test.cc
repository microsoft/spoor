// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <limits>

#include "gmock/gmock.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/runtime/runtime.h"

namespace {

using testing::MockFunction;

TEST(Runtime, Initialize) {  // NOLINT
  ASSERT_FALSE(_spoor_runtime_RuntimeInitialized());
  _spoor_runtime_InitializeRuntime();
  ASSERT_FALSE(_spoor_runtime_RuntimeInitialized());
  _spoor_runtime_DeinitializeRuntime();
  ASSERT_FALSE(_spoor_runtime_RuntimeInitialized());
}

TEST(Runtime, Enable) {  // NOLINT
  ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
  _spoor_runtime_EnableRuntime();
  ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
  _spoor_runtime_DisableRuntime();
  ASSERT_FALSE(_spoor_runtime_RuntimeEnabled());
}

TEST(Runtime, LogEvent) {  // NOLINT
  _spoor_runtime_LogEventWithTimestamp(1, 2, 3, 4);
  _spoor_runtime_LogEvent(1, 2, 3);
  _spoor_runtime_LogFunctionEntry(1);
  _spoor_runtime_LogFunctionExit(1);
}

namespace flush_trace_events_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void()> callback_{};

TEST(Runtime, FlushTraceEvents) {  // NOLINT
  _spoor_runtime_FlushTraceEvents({});

  MockFunction<void()> callback{};
  callback_ = callback.AsStdFunction();
  EXPECT_CALL(callback, Call());
  _spoor_runtime_FlushTraceEvents([] { callback_(); });
}

}  // namespace flush_trace_events_test

TEST(Runtime, ClearTraceEvents) {  // NOLINT
  _spoor_runtime_ClearTraceEvents();
}

namespace flushed_trace_files_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(_spoor_runtime_TraceFiles)> callback_{};

TEST(Runtime, FlushedTraceFiles) {  // NOLINT
  _spoor_runtime_FlushedTraceFiles({});

  constexpr _spoor_runtime_TraceFiles expected_trace_files{
      .file_paths_size = 0, .file_path_sizes = nullptr, .file_paths = nullptr};
  MockFunction<void(_spoor_runtime_TraceFiles)> callback{};
  callback_ = callback.AsStdFunction();
  EXPECT_CALL(callback, Call(expected_trace_files));
  _spoor_runtime_FlushedTraceFiles(
      [](auto trace_files) { callback_(trace_files); });
}

}  // namespace flushed_trace_files_test

namespace delete_flushed_trace_files_older_than_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(_spoor_runtime_DeletedFilesInfo)> callback_{};

TEST(Runtime, DeleteFlushedTraceFilesOlderThan) {  // NOLINT
  _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
      std::numeric_limits<_spoor_runtime_SystemTimestampSeconds>::min(), {});

  constexpr _spoor_runtime_DeletedFilesInfo expected_deleted_files_info{
      .deleted_files = 0, .deleted_bytes = 0};
  MockFunction<void(_spoor_runtime_DeletedFilesInfo)> callback{};
  callback_ = callback.AsStdFunction();
  EXPECT_CALL(callback, Call(expected_deleted_files_info));
  _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
      std::numeric_limits<_spoor_runtime_SystemTimestampSeconds>::min(),
      [](auto deleted_files_info) { callback_(deleted_files_info); });
}

}  // namespace delete_flushed_trace_files_older_than_test

TEST(Runtime, GetConfig) {  // NOLINT
  constexpr _spoor_runtime_Config expected_config{
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

TEST(Runtime, StubImplementation) {  // NOLINT
  ASSERT_TRUE(_spoor_runtime_StubImplementation());
}

}  // namespace
