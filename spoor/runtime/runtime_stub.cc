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

auto _spoor_runtime_LogFunctionEntry(
    const spoor::runtime::FunctionId /*unused*/) -> void {}

auto _spoor_runtime_LogFunctionExit(const spoor::runtime::FunctionId /*unused*/)
    -> void {}
