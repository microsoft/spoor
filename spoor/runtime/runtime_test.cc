// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#include <future>
#include <limits>

#include "gsl/gsl"
#include "gtest/gtest.h"

namespace {

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
  _spoor_runtime_InitializeRuntime();
  _spoor_runtime_FlushTraceEvents({});
}

TEST(Runtime, ClearTraceEvents) {  // NOLINT
  _spoor_runtime_ClearTraceEvents();
  _spoor_runtime_InitializeRuntime();
  _spoor_runtime_ClearTraceEvents();
}

namespace flused_trace_files_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::promise<_spoor_runtime_TraceFiles> promise_{};

TEST(Runtime, FlushedTraceFiles) {  // NOLINT
  _spoor_runtime_FlushedTraceFiles(
      [](auto trace_files) { promise_.set_value(trace_files); });
  auto future = promise_.get_future();
  auto trace_files = future.get();
  auto _ =
      gsl::finally([&] { _spoor_runtime_ReleaseTraceFilePaths(&trace_files); });
  ASSERT_EQ(trace_files.size, 0);
}

}  // namespace flused_trace_files_test

namespace delete_flushed_trace_files_older_than_test {

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::promise<_spoor_runtime_DeletedFilesInfo> promise_{};

TEST(Runtime, DeleteFlushedTraceFilesOlderThan) {  // NOLINT
  _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
      std::numeric_limits<_spoor_runtime_SystemTimestampSeconds>::max(),
      [](auto deleted_files_info) { promise_.set_value(deleted_files_info); });
  auto future = promise_.get_future();
  const auto deletes_files_info = future.get();
  ASSERT_EQ(deletes_files_info.deleted_files, 0);
  ASSERT_EQ(deletes_files_info.deleted_bytes, 0);
}

}  // namespace delete_flushed_trace_files_older_than_test

TEST(Runtime, GetConfig) {  // NOLINT
  const auto config = _spoor_runtime_GetConfig();
  ASSERT_NE(config.trace_file_path, nullptr);
}

}  // namespace
