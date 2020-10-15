#pragma once

#include "util/numeric.h"

#ifdef __cplusplus
extern "C" {
#endif

// NOLINTNEXTLINE(readability-identifier-naming)
struct __spoor_runtime_TraceFiles {
  uint64 size;
  char* file_path;
};

// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef uint64 __spoor_runtime_FunctionId;
// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef uint64 __spoor_runtime_SystemTimestampSeconds;
// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef void (*__spoor_runtime_FlushTraceEventsCallback)(
    uint32 /* Number of flushed files */, uint64 /* Number of flushed bytes */);
// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef void (*__spoor_runtime_FlushedTraceEventsCallback)(
    __spoor_runtime_TraceFiles);
// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef void (*__spoor_runtime_DeleteFlushedTraceFilesCallback)(
    uint32 /* Number of deleted files */, uint64 /* Number of deleted bytes */);

// Initialize the event tracing runtime. By default a call to this function is
// inserted at the start of `main` by the compile-time instrumentation.
// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_InitRuntime() -> void;

// Deinitialize the event tracing runtime. By default a call to this function
// is inserted at the  end of `main` by the compile-time instrumentation.
// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_DeinitRuntime() -> void;

// Check if the event tracing runtiem is initialized.
// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_RuntimeInitialized() -> bool;

// Enable runtime logging.
// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_EnableRuntime() -> void;

// Disable runtime logging.
// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_DisableRuntime() -> void;

// Check if runtime logging is enabled.
// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_RuntimeEnabled() -> bool;

// Log that the program entered a function. The function internally checks if
// the runtime is enabled before collecting the current timestamp and logging
// the event. A call to this function is inserted at the start of every function
// by the compile-time instrumentation.
// NOLINTNEXTLINE(readability-identifier-naming)
inline auto __spoor_runtime_LogFunctionEntry(
    __spoor_runtime_FunctionId function_id) -> void;

// Log that the program exited a function. The function internally checks if the
// runtime is enabled before collecting the current timestamp and logging the
// event. A call to this function is inserted at the end of every function by
// the compile-time instrumentation.
// NOLINTNEXTLINE(readability-identifier-naming)
inline auto __spoor_runtime_LogFunctionExit(
    __spoor_runtime_FunctionId function_id) -> void;

// Flush in-memory trace events to disk.
// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_FlushTraceEvents(
    __spoor_runtime_FlushTraceEventsCallback callback) -> void;

// Clear the trace events from memory without flushing them to disk.
// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_ClearTraceEvents() -> void;

// Retrieve an array of all trace files on disk. Callers are responsible for
// freeing the array’s memory.
// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_FlushedTraceFiles(
    __spoor_runtime_FlushedTraceEventsCallback callback) -> void;

// Delete all trace files older than a given timestamp.
// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_DeleteFlushedTraceFilesOlderThan(
    __spoor_runtime_SystemTimestampSeconds system_timestamp,
    __spoor_runtime_DeleteFlushedTraceFilesCallback callback) -> void;

#ifdef __cplusplus
}
#endif
