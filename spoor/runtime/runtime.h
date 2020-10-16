#ifndef SPOOR_RUNTIME
#define SPOOR_RUNTIME

#ifdef __cplusplus
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

// NOLINTNEXTLINE(readability-identifier-naming)
typedef struct __spoor_runtime_TraceFiles {
  int32_t size;
  // NOTE: The caller takes ownership of the file_paths array and is responsible
  // for releasing the memory.
  char** file_paths;
} __spoor_runtime_TraceFiles;

// NOLINTNEXTLINE(readability-identifier-naming)
typedef struct __spoor_runtime_DeletedFilesInfo {
  int32_t deleted_files;
  int64_t deleted_bytes;
} __spoor_runtime_DeletedFilesInfo;

// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef uint64_t __spoor_runtime_FunctionId;
// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef int64_t __spoor_runtime_SystemTimestampSeconds;
// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef void (*__spoor_runtime_FlushTraceEventsCallback)();  // TODO
//    uint32_t /* Number of flushed files */,
//    uint64_t /* Number of flushed bytes */);
// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef void (*__spoor_runtime_ClearTraceEventsCallback)();  // TODO
//    uint64_t /* Number of cleared bytes */);
// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef void (*__spoor_runtime_FlushedTraceEventsCallback)(
    __spoor_runtime_TraceFiles);
// NOLINTNEXTLINE(modernize-use-using, readability-identifier-naming)
typedef void (*__spoor_runtime_DeleteFlushedTraceFilesCallback)(
    __spoor_runtime_DeletedFilesInfo);

// Initialize the event tracing runtime. A call to this function is inserted at
// the start of `main` by the compile-time instrumentation unless configured
// otherwise. NOLINTNEXTLINE(readability-identifier-naming)
void __spoor_runtime_InitializeRuntime();

// Deinitialize the event tracing runtime. A call to this function is inserted
// at the  end of `main` by the compile-time instrumentation.
// NOLINTNEXTLINE(readability-identifier-naming)
void __spoor_runtime_DeinitializeRuntime();

// Check if the event tracing runtiem is initialized.
// NOLINTNEXTLINE(readability-identifier-naming)
bool __spoor_runtime_RuntimeInitialized();

// Enable runtime logging.
// NOLINTNEXTLINE(readability-identifier-naming)
void __spoor_runtime_EnableRuntime();

// Disable runtime logging.
// NOLINTNEXTLINE(readability-identifier-naming)
void __spoor_runtime_DisableRuntime();

// Check if runtime logging is enabled.
// NOLINTNEXTLINE(readability-identifier-naming)
bool __spoor_runtime_RuntimeEnabled();

// Log that the program entered a function. The function internally checks if
// the runtime is enabled before collecting the current timestamp and logging
// the event. A call to this function is inserted at the start of every function
// by the compile-time instrumentation.
// NOLINTNEXTLINE(readability-identifier-naming)
void __spoor_runtime_LogFunctionEntry(
    __spoor_runtime_FunctionId function_id);

// Log that the program exited a function. The function internally checks if the
// runtime is enabled before collecting the current timestamp and logging the
// event. A call to this function is inserted at the end of every function by
// the compile-time instrumentation.
// NOLINTNEXTLINE(readability-identifier-naming)
void __spoor_runtime_LogFunctionExit(
    __spoor_runtime_FunctionId function_id);

// Flush in-memory trace events to disk.
// NOLINTNEXTLINE(readability-identifier-naming)
void __spoor_runtime_FlushTraceEvents(
    __spoor_runtime_FlushTraceEventsCallback callback);

// Clear the trace events from memory without flushing them to disk.
// NOLINTNEXTLINE(readability-identifier-naming)
void __spoor_runtime_ClearTraceEvents(
    __spoor_runtime_FlushTraceEventsCallback callback);

// Retrieve an array of all trace files on disk.
// NOLINTNEXTLINE(readability-identifier-naming)
void __spoor_runtime_FlushedTraceFiles(
    __spoor_runtime_FlushedTraceEventsCallback callback);

// Delete all trace files older than a given timestamp.
// NOLINTNEXTLINE(readability-identifier-naming)
void __spoor_runtime_DeleteFlushedTraceFilesOlderThan(
    __spoor_runtime_SystemTimestampSeconds system_timestamp,
    __spoor_runtime_DeleteFlushedTraceFilesCallback callback);

#ifdef __cplusplus
}
#endif

#endif
