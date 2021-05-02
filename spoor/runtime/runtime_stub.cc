// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <cstdint>
#include <filesystem>
#include <functional>
#include <thread>
#include <vector>

#include "spoor/runtime/runtime.h"

namespace spoor::runtime {

auto Initialize() -> void {}

auto Deinitialize() -> void {}

auto Initialized() -> bool { return false; }

auto Enable() -> void {}

auto Disable() -> void {}

auto Enabled() -> bool { return false; }

auto LogEvent(const EventType /*unused*/, const TimestampNanoseconds /*unused*/,
              const uint64_t /*unused*/, const uint64_t /*unused*/) -> void {}

auto LogEvent(const EventType /*unused*/, const uint64_t /*unused*/,
              const uint64_t /*unused*/) -> void {}

auto LogFunctionEntry(const FunctionId /*unused*/) -> void {}

auto LogFunctionExit(const FunctionId /*unused*/) -> void {}

auto FlushTraceEvents(std::function<void()> callback) -> void {
  if (callback == nullptr) return;
  std::thread{std::move(callback)}.detach();
}

auto ClearTraceEvents() -> void {}

auto FlushedTraceFiles(
    std::function<void(std::vector<std::filesystem::path>)> callback) -> void {
  if (callback == nullptr) return;
  std::thread{std::move(callback), std::vector<std::filesystem::path>{}}
      .detach();
}

auto DeleteFlushedTraceFilesOlderThan(
    const SystemTimestampSeconds /*unused*/,
    std::function<void(DeletedFilesInfo)> callback) -> void {
  if (callback == nullptr) return;
  std::thread{std::move(callback),
              DeletedFilesInfo{.deleted_files = 0, .deleted_bytes = 0}}
      .detach();
}

auto GetConfig() -> Config {
  return {.trace_file_path = {},
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

auto StubImplementation() -> bool { return true; }

}  // namespace spoor::runtime

auto _spoor_runtime_Initialize() -> void {}

auto _spoor_runtime_Deinitialize() -> void {}

auto _spoor_runtime_Initialized() -> bool { return false; }

auto _spoor_runtime_Enable() -> void {}

auto _spoor_runtime_Disable() -> void {}

auto _spoor_runtime_Enabled() -> bool { return false; }

auto _spoor_runtime_LogEventWithTimestamp(
    const _spoor_runtime_EventType /*unused*/,
    const _spoor_runtime_TimestampNanoseconds /*unused*/,
    const uint64_t /*unused*/, const uint32_t /*unused*/) -> void {}

auto _spoor_runtime_LogEvent(const _spoor_runtime_EventType /*unused*/,
                             const uint64_t /*unused*/,
                             const uint32_t /*unused*/) -> void {}

auto _spoor_runtime_LogFunctionEntry(const _spoor_runtime_FunctionId /*unused*/)
    -> void {}

auto _spoor_runtime_LogFunctionExit(const _spoor_runtime_FunctionId /*unused*/)
    -> void {}

auto _spoor_runtime_FlushTraceEvents(
    const _spoor_runtime_FlushTraceEventsCallback callback) -> void {
  auto callback_adapter = [callback]() {
    if (callback == nullptr) return;
    callback();
  };
  std::thread{std::move(callback_adapter)}.detach();
}

auto _spoor_runtime_ClearTraceEvents() -> void {}

auto _spoor_runtime_FlushedTraceFiles(
    const _spoor_runtime_FlushedTraceFilesCallback callback) -> void {
  auto callback_adapter = [callback]() {
    if (callback == nullptr) return;
    callback({.file_paths_size = 0,
              .file_path_sizes = nullptr,
              .file_paths = nullptr});
  };
  std::thread{std::move(callback_adapter)}.detach();
}

auto _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
    const _spoor_runtime_SystemTimestampSeconds /*unused*/,
    const _spoor_runtime_DeleteFlushedTraceFilesCallback callback) -> void {
  auto callback_adapter = [callback]() {
    if (callback == nullptr) return;
    callback({.deleted_files = 0, .deleted_bytes = 0});
  };
  std::thread{std::move(callback_adapter)}.detach();
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
