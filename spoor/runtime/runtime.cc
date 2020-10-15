#include "spoor/runtime/runtime.h"

#include <sys/types.h>
#include <unistd.h>

#include "buffer/circular_slice_buffer.h"
#include "spoor/runtime/config/config.h"
#include "spoor/runtime/event_logger/event_logger.h"
#include "spoor/runtime/flush_queue/disk_flush_queue.h"
#include "spoor/runtime/runtime_manager/runtime_manager.h"
#include "spoor/runtime/trace/trace_writer.h"

namespace spoor::runtime {

const auto user_options_ = config::UserOptions::FromEnv();
util::time::SystemClock system_clock_{};
util::time::SteadyClock steady_clock_{};
trace::TraceFileWriter trace_writer_{};
flush_queue::DiskFlushQueue flush_queue_{
    {.trace_file_path = user_options_.trace_file_path,
     .buffer_retention_duration =
         std::chrono::nanoseconds{
             user_options_.event_buffer_retention_duration_nanoseconds},
     .system_clock = &system_clock_,
     .steady_clock = &steady_clock_,
     .trace_writer = &trace_writer_,
     .session_id = user_options_.session_id,
     .process_id = ::getpid(),
     .max_buffer_flush_attempts =
         user_options_.max_flush_buffer_to_file_attempts,
     .flush_immediately =
         user_options_.flush_event_buffer_immediately_after_flush}};
runtime_manager::RuntimeManager runtime_{
    {.trace_file_path = user_options_.trace_file_path,
     .system_clock = &system_clock_,
     .steady_clock = &steady_clock_,
     .session_id = user_options_.session_id,
     .process_id = ::getpid(),
     .reserved_pool_capacity = user_options_.reserved_event_pool_capacity,
     .reserved_pool_max_slice_capacity =
         user_options_.max_reserved_event_buffer_slice_capacity,
     .dynamic_pool_capacity = user_options_.dynamic_event_pool_capacity,
     .dynamic_pool_max_slice_capacity =
         user_options_.max_dynamic_event_buffer_slice_capacity}};
thread_local event_logger::EventLogger event_logger_{
    {.steady_clock = &steady_clock_,
     .event_logger_notifier = &runtime_,
     .flush_queue = &flush_queue_,
     .capacity = user_options_.thread_event_buffer_capacity}};

}  // namespace spoor::runtime

namespace {

using spoor::runtime::event_logger_;
using spoor::runtime::runtime_;

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_InitRuntime() -> void { runtime_.Initialize(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_DeinitRuntime() -> void { runtime_.Deinitialize(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_RuntimeInitialized() -> bool { return runtime_.Initialized(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_EnableRuntime() -> void { runtime_.Enable(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_DisableRuntime() -> void { runtime_.Disable(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_RuntimeEnabled() -> bool { return runtime_.Enabled(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_LogFunctionEntry(__spoor_runtime_FunctionId function_id)
    -> void {
  if (!runtime_.Enabled()) return;
  event_logger_.LogFunctionEntry(function_id);
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_LogFunctionExit(__spoor_runtime_FunctionId function_id)
    -> void {
  if (!runtime_.Enabled()) return;
  event_logger_.LogFunctionExit(function_id);
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_FlushTraceEvents(
    __spoor_runtime_FlushTraceEventsCallback callback) -> void {
  (void)callback;  // TODO
  runtime_.Flush();
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_ClearTraceEvents() -> void { runtime_.Clear(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_FlushedTraceFiles(
    __spoor_runtime_FlushedTraceEventsCallback callback) -> void {
  (void)callback;  // TODO
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_DeleteFlushedTraceFilesOlderThan(
    __spoor_runtime_SystemTimestampSeconds system_timestamp,
    __spoor_runtime_DeleteFlushedTraceFilesCallback callback) -> void {
  (void)system_timestamp;  // TODO
  (void)callback;  // TODO
}
