// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// This header provides C++ and C APIs to interface with Spoor's runtime engine.

#ifndef SPOOR_RUNTIME_RUNTIME_H_
#define SPOOR_RUNTIME_RUNTIME_H_

#ifdef __cplusplus
#include <cstdint>
#include <filesystem>
#include <functional>
#include <vector>
#else
#include <stdint.h>
#endif

// C++ API

#ifdef __cplusplus
namespace spoor::runtime {

using DurationNanoseconds = int64_t;
using EventType = uint32_t;
using FunctionId = uint64_t;
using SessionId = uint64_t;
using SizeType = uint64_t;
using SystemTimestampSeconds = int64_t;
using TimestampNanoseconds = int64_t;

struct DeletedFilesInfo {
  int32_t deleted_files;
  int64_t deleted_bytes;
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
  int32_t max_flush_buffer_to_file_attempts;
  bool flush_all_events;
};

// Initialize the event tracing runtime. A call to this function is inserted at
// the start of `main` by the compile-time instrumentation unless configured
// otherwise.
auto Initialize() -> void;

// Deinitialize the event tracing runtime. A call to this function is inserted
// at the  end of `main` by the compile-time instrumentation.
auto Deinitialize() -> void;

// Check if the runtime engine is initialized.
auto Initialized() -> bool;

// Enable runtime logging.
auto Enable() -> void;

// Disable runtime logging.
auto Disable() -> void;

// Check if runtime logging is enabled.
auto Enabled() -> bool;

// Log that the program generated an event. The function internally checks if
// the runtime is enabled before logging the event.
auto LogEvent(EventType event, TimestampNanoseconds steady_clock_timestamp,
              uint64_t payload_1, uint64_t payload_2) -> void;

// Log that the program generated an event. The function internally checks if
// the runtime is enabled before collecting the current timestamp and logging
// the event.
auto LogEvent(EventType event, uint64_t payload_1, uint64_t payload_2) -> void;

// Log that the program entered a function. The function internally checks if
// the runtime is enabled before collecting the current timestamp and logging
// the event. A call to this function is inserted at the start of every function
// by the compile-time instrumentation.
auto LogFunctionEntry(FunctionId function_id) -> void;

// Log that the program exited a function. The function internally checks if the
// runtime is enabled before collecting the current timestamp and logging the
// event. A call to this function is inserted at the end of every function by
// the compile-time instrumentation.
auto LogFunctionExit(FunctionId function_id) -> void;

// Flush in-memory trace events to disk. The callback is invoked on a background
// thread.
auto FlushTraceEvents(std::function<void()> callback) -> void;

// Clear the trace events from memory without flushing them to disk.
auto ClearTraceEvents() -> void;

// Retrieve an array of all trace files on disk. The callback is invoked on a
// background thread.
auto FlushedTraceFiles(
    std::function<void(std::vector<std::filesystem::path>)> callback) -> void;

// Delete all trace files older than a given timestamp. The callback is invoked
// on a background thread.
auto DeleteFlushedTraceFilesOlderThan(
    SystemTimestampSeconds system_timestamp,
    std::function<void(DeletedFilesInfo)> callback) -> void;

// Retrieve Spoor's configuration.
auto GetConfig() -> Config;

// Check if the runtime contains stub implementations.
auto StubImplementation() -> bool;

}  // namespace spoor::runtime
#endif  // __cplusplus

// C API

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t _spoor_runtime_DurationNanoseconds;
typedef uint32_t _spoor_runtime_EventType;
typedef uint64_t _spoor_runtime_FunctionId;
typedef uint64_t _spoor_runtime_SessionId;
typedef uint64_t _spoor_runtime_SizeType;
typedef int64_t _spoor_runtime_SystemTimestampSeconds;
typedef int64_t _spoor_runtime_TimestampNanoseconds;

typedef struct _spoor_runtime_TraceFiles {
  _spoor_runtime_SizeType file_paths_size;
  // NOTE: The caller takes ownership of `file_path_sizes` and `file_paths`, and
  // is responsible for releasing the memory, e.g., by calling
  // `_spoor_runtime_ReleaseTraceFilePaths`.
  _spoor_runtime_SizeType* file_path_sizes;
  char** file_paths;
} _spoor_runtime_TraceFiles;

typedef struct _spoor_runtime_DeletedFilesInfo {
  int32_t deleted_files;
  int64_t deleted_bytes;
} _spoor_runtime_DeletedFilesInfo;

typedef struct _spoor_runtime_Config {
  _spoor_runtime_SizeType trace_file_path_size;
  const char* trace_file_path;  // Non-owning
  _spoor_runtime_SessionId session_id;
  _spoor_runtime_SizeType thread_event_buffer_capacity;
  _spoor_runtime_SizeType max_reserved_event_buffer_slice_capacity;
  _spoor_runtime_SizeType max_dynamic_event_buffer_slice_capacity;
  _spoor_runtime_SizeType reserved_event_pool_capacity;
  _spoor_runtime_SizeType dynamic_event_pool_capacity;
  _spoor_runtime_SizeType dynamic_event_slice_borrow_cas_attempts;
  _spoor_runtime_DurationNanoseconds
      event_buffer_retention_duration_nanoseconds;
  int32_t max_flush_buffer_to_file_attempts;
  bool flush_all_events;
} _spoor_runtime_Config;

typedef void (*_spoor_runtime_FlushTraceEventsCallback)();
typedef void (*_spoor_runtime_FlushedTraceFilesCallback)(
    _spoor_runtime_TraceFiles);
typedef void (*_spoor_runtime_DeleteFlushedTraceFilesCallback)(
    _spoor_runtime_DeletedFilesInfo);

// Initialize the event tracing runtime. A call to this function is inserted at
// the start of `main` by the compile-time instrumentation unless configured
// otherwise.
void _spoor_runtime_Initialize();

// Deinitialize the event tracing runtime. A call to this function is inserted
// at the  end of `main` by the compile-time instrumentation.
void _spoor_runtime_Deinitialize();

// Check if the runtime engine is initialized.
bool _spoor_runtime_Initialized();

// Enable runtime logging.
void _spoor_runtime_Enable();

// Disable runtime logging.
void _spoor_runtime_Disable();

// Check if runtime logging is enabled.
bool _spoor_runtime_Enabled();

// Log that the program generated an event. The function internally checks if
// the runtime is enabled before logging the event.
void _spoor_runtime_LogEventWithTimestamp(
    _spoor_runtime_EventType event,
    _spoor_runtime_TimestampNanoseconds steady_clock_timestamp,
    uint64_t payload_1, uint32_t payload_2);

// Log that the program generated an event. The function internally checks if
// the runtime is enabled before collecting the current timestamp and logging
// the event.
void _spoor_runtime_LogEvent(_spoor_runtime_EventType event, uint64_t payload_1,
                             uint32_t payload_2);

// Log that the program entered a function. The function internally checks if
// the runtime is enabled before collecting the current timestamp and logging
// the event. A call to this function is inserted at the start of every function
// by the compile-time instrumentation.
void _spoor_runtime_LogFunctionEntry(_spoor_runtime_FunctionId function_id);

// Log that the program exited a function. The function internally checks if the
// runtime is enabled before collecting the current timestamp and logging the
// event. A call to this function is inserted at the end of every function by
// the compile-time instrumentation.
void _spoor_runtime_LogFunctionExit(_spoor_runtime_FunctionId function_id);

// Flush in-memory trace events to disk. The callback is invoked on a background
// thread.
void _spoor_runtime_FlushTraceEvents(
    _spoor_runtime_FlushTraceEventsCallback callback);

// Clear the trace events from memory without flushing them to disk. The
// callback is invoked on a background thread;
void _spoor_runtime_ClearTraceEvents();

// Retrieve an array of all trace files on disk. The callback is invoked on a
// background thread.
void _spoor_runtime_FlushedTraceFiles(
    _spoor_runtime_FlushedTraceFilesCallback callback);

// Delete all trace files older than a given timestamp. The callback is invoked
// on a background thread;
void _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
    _spoor_runtime_SystemTimestampSeconds system_timestamp,
    _spoor_runtime_DeleteFlushedTraceFilesCallback callback);

// Retrieve Spoor's configuration.
_spoor_runtime_Config _spoor_runtime_GetConfig();

// Check if the runtime contains stub implementations.
bool _spoor_runtime_StubImplementation();

// Release the memory owned by a `_spoor_runtime_TraceFiles` object (but not
// the object itself).
// Implemented in the stub.
void _spoor_runtime_ReleaseTraceFilePaths(
    _spoor_runtime_TraceFiles* trace_files);

// Equality.
// Implemented in the stub.
bool _spoor_runtime_DeletedFilesInfoEqual(
    const _spoor_runtime_DeletedFilesInfo* lhs,
    const _spoor_runtime_DeletedFilesInfo* rhs);
bool _spoor_runtime_TraceFilesEqual(const _spoor_runtime_TraceFiles* lhs,
                                    const _spoor_runtime_TraceFiles* rhs);
bool _spoor_runtime_ConfigEqual(const _spoor_runtime_Config* lhs,
                                const _spoor_runtime_Config* rhs);
#ifdef __cplusplus
}  // extern "C"
#endif

// Equality.
// Implemented in the stub.
#ifdef __cplusplus
namespace spoor::runtime {

auto operator==(const DeletedFilesInfo& lhs, const DeletedFilesInfo& rhs)
    -> bool;
auto operator==(const Config& lhs, const Config& rhs) -> bool;

}  // namespace spoor::runtime

auto operator==(const _spoor_runtime_DeletedFilesInfo& lhs,
                const _spoor_runtime_DeletedFilesInfo& rhs) -> bool;
auto operator==(const _spoor_runtime_TraceFiles& lhs,
                const _spoor_runtime_TraceFiles& rhs) -> bool;
auto operator==(const _spoor_runtime_Config& lhs,
                const _spoor_runtime_Config& rhs) -> bool;
#endif

#endif  // SPOOR_RUNTIME_RUNTIME_H_
