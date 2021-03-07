// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

auto _spoor_runtime_InitializeRuntime() -> void {}

auto _spoor_runtime_DeinitializeRuntime() -> void {}

auto _spoor_runtime_RuntimeInitialized() -> bool { return false; }

auto _spoor_runtime_EnableRuntime() -> void {}

auto _spoor_runtime_DisableRuntime() -> void {}

auto _spoor_runtime_RuntimeEnabled() -> bool { return false; }

auto _spoor_runtime_LogFunctionEntry(const _spoor_runtime_FunctionId /*unused*/)
    -> void {}

auto _spoor_runtime_LogFunctionExit(const _spoor_runtime_FunctionId /*unused*/)
    -> void {}

auto _spoor_runtime_FlushTraceEvents(
    const _spoor_runtime_FlushTraceEventsCallback callback) -> void {
  if (callback == nullptr) return;
  callback();
}

auto _spoor_runtime_ClearTraceEvents() -> void {}

auto _spoor_runtime_FlushedTraceFiles(
    const _spoor_runtime_FlushedTraceFilesCallback callback) -> void {
  if (callback == nullptr) return;
  callback({.file_paths = nullptr});
}

auto _spoor_runtime_ReleaseTraceFilePaths(_spoor_runtime_TraceFiles*
                                          /*unused*/) -> void {}

auto _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
    const _spoor_runtime_SystemTimestampSeconds /*unused*/,
    const _spoor_runtime_DeleteFlushedTraceFilesCallback callback) -> void {
  if (callback == nullptr) return;
  callback({.deleted_files = 0, .deleted_bytes = 0});
}

auto _spoor_runtime_GetConfig() -> _spoor_runtime_Config {
  return {.trace_file_path = nullptr,
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
}

auto _spoor_runtime_StubImplementation() -> bool { return true; }
