// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// This header provides C++ and C APIs to interface with Spoor's runtime engine.

#ifndef SPOOR_RUNTIME_RUNTIME_H_
#define SPOOR_RUNTIME_RUNTIME_H_

#ifndef SPOOR_RUNTIME_EXPORT
// Clang Tidy false positive. NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define SPOOR_RUNTIME_EXPORT __attribute__((visibility("default")))
#endif

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

// Configuration
// Implement these functions to configure Spoor's runtime. Return `std::nullopt`
// or link against `libspoor_runtime_default_config.a` to use the default
// config. Configuration implementations are never instrumented.
namespace spoor::runtime {

// Returns the path to Spoor's runtime config file.
SPOOR_RUNTIME_EXPORT extern auto ConfigFilePath() -> std::optional<std::string>;

}  // namespace spoor::runtime

// Runtime API
namespace spoor::runtime {

using DurationNanoseconds = std::int64_t;
using EventType = std::uint32_t;
using FunctionId = std::uint64_t;
using SessionId = std::uint64_t;
using SizeType = std::uint64_t;
using SystemTimestampSeconds = std::int64_t;
using TimestampNanoseconds = std::int64_t;

struct DeletedFilesInfo {
  std::int32_t deleted_files;
  std::int64_t deleted_bytes;
};

struct Config {
  std::filesystem::path trace_file_path;
  SessionId session_id;
  SizeType thread_event_buffer_capacity;
  SizeType max_reserved_event_buffer_slice_capacity;
  SizeType max_dynamic_event_buffer_slice_capacity;
  SizeType reserved_event_pool_capacity;
  SizeType dynamic_event_pool_capacity;
  SizeType dynamic_event_slice_borrow_cas_attempts;
  DurationNanoseconds event_buffer_retention_duration_nanoseconds;
  std::int32_t max_flush_buffer_to_file_attempts;
  bool flush_all_events;
};

// Initialize the event tracing runtime. A call to this function is inserted at
// the start of `main` by the compile-time instrumentation unless configured
// otherwise.
SPOOR_RUNTIME_EXPORT auto Initialize() -> void;

// Deinitialize the event tracing runtime. A call to this function is inserted
// at the  end of `main` by the compile-time instrumentation.
SPOOR_RUNTIME_EXPORT auto Deinitialize() -> void;

// Check if the runtime engine is initialized.
SPOOR_RUNTIME_EXPORT auto Initialized() -> bool;

// Enable runtime logging.
SPOOR_RUNTIME_EXPORT auto Enable() -> void;

// Disable runtime logging.
SPOOR_RUNTIME_EXPORT auto Disable() -> void;

// Check if runtime logging is enabled.
SPOOR_RUNTIME_EXPORT auto Enabled() -> bool;

// Log that the program generated an event. The function internally checks if
// the runtime is enabled before logging the event.
SPOOR_RUNTIME_EXPORT auto LogEvent(EventType event,
                                   TimestampNanoseconds steady_clock_timestamp,
                                   std::uint64_t payload_1,
                                   std::uint64_t payload_2) -> void;

// Log that the program generated an event. The function internally checks if
// the runtime is enabled before collecting the current timestamp and logging
// the event.
SPOOR_RUNTIME_EXPORT auto LogEvent(EventType event, std::uint64_t payload_1,
                                   std::uint64_t payload_2) -> void;

// Flush in-memory trace events to disk. The callback is invoked on a background
// thread.
SPOOR_RUNTIME_EXPORT auto FlushTraceEvents(std::function<void()> callback)
    -> void;

// Clear the trace events from memory without flushing them to disk.
SPOOR_RUNTIME_EXPORT auto ClearTraceEvents() -> void;

// Retrieve an array of all trace files on disk. The callback is invoked on a
// background thread.
SPOOR_RUNTIME_EXPORT auto FlushedTraceFiles(
    std::function<void(std::vector<std::filesystem::path>)> callback) -> void;

// Delete all trace files older than a given timestamp. The callback is invoked
// on a background thread.
SPOOR_RUNTIME_EXPORT auto DeleteFlushedTraceFilesOlderThan(
    SystemTimestampSeconds system_timestamp,
    std::function<void(DeletedFilesInfo)> callback) -> void;

// Retrieve Spoor's configuration.
SPOOR_RUNTIME_EXPORT auto GetConfig() -> Config;

// Check if the runtime contains stub implementations.
SPOOR_RUNTIME_EXPORT auto StubImplementation() -> bool;

// Equality.
// Implemented in the stub.

auto operator==(const DeletedFilesInfo& lhs, const DeletedFilesInfo& rhs)
    -> bool;
auto operator==(const Config& lhs, const Config& rhs) -> bool;

}  // namespace spoor::runtime

// Internal.
// Exposed for use by Spoor's instrumentation.
extern "C" {

// NOLINTNEXTLINE(readability-identifier-naming)
using _spoor_runtime_FunctionId = spoor::runtime::FunctionId;

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_Initialize() -> void;

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_Deinitialize() -> void;

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_Enable() -> void;

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_Disable() -> void;

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_LogFunctionEntry(spoor::runtime::FunctionId function_id)
    -> void;

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_LogFunctionExit(spoor::runtime::FunctionId function_id)
    -> void;

}  // extern "C"

#endif  // SPOOR_RUNTIME_RUNTIME_H_
