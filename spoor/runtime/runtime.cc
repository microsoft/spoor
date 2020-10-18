#include "spoor/runtime/runtime.h"

#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <utility>

#include "gsl/gsl"
#include "spoor/runtime/config/config.h"
#include "spoor/runtime/flush_queue/disk_flush_queue.h"
#include "spoor/runtime/runtime_manager/runtime_manager.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_writer.h"
#include "util/numeric.h"

namespace {

const auto config_ = spoor::runtime::config::Config::FromEnv();
util::time::SystemClock system_clock_{};
util::time::SteadyClock steady_clock_{};
spoor::runtime::trace::TraceFileWriter trace_writer_{};
spoor::runtime::flush_queue::DiskFlushQueue flush_queue_{
    {.trace_file_path = config_.trace_file_path,
     .buffer_retention_duration =
         std::chrono::nanoseconds{
             config_.event_buffer_retention_duration_nanoseconds},
     .system_clock = &system_clock_,
     .steady_clock = &steady_clock_,
     .trace_writer = &trace_writer_,
     .session_id = config_.session_id,
     .process_id = ::getpid(),
     .max_buffer_flush_attempts = config_.max_flush_buffer_to_file_attempts,
     .flush_all_events = config_.flush_all_events}};
spoor::runtime::runtime_manager::RuntimeManager runtime_{
    {.steady_clock = &steady_clock_,
     .flush_queue = &flush_queue_,
     .thread_event_buffer_capacity = config_.thread_event_buffer_capacity,
     .reserved_pool_capacity = config_.reserved_event_pool_capacity,
     .reserved_pool_max_slice_capacity =
         config_.max_reserved_event_buffer_slice_capacity,
     .dynamic_pool_capacity = config_.dynamic_event_pool_capacity,
     .dynamic_pool_max_slice_capacity =
         config_.max_dynamic_event_buffer_slice_capacity,
     .flush_all_events = config_.flush_all_events}};

}  // namespace

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_InitializeRuntime() -> void { runtime_.Initialize(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_DeinitializeRuntime() -> void { runtime_.Deinitialize(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_RuntimeInitialized() -> bool {
  return runtime_.Initialized();
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_EnableRuntime() -> void { runtime_.Enable(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_DisableRuntime() -> void { runtime_.Disable(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_RuntimeEnabled() -> bool { return runtime_.Enabled(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_LogFunctionEntry(
    const __spoor_runtime_FunctionId function_id) -> void {
  runtime_.LogEvent(spoor::runtime::trace::Event::Type::kFunctionEntry,
                    function_id);
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_LogFunctionExit(
    const __spoor_runtime_FunctionId function_id) -> void {
  runtime_.LogEvent(spoor::runtime::trace::Event::Type::kFunctionExit,
                    function_id);
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_FlushTraceEvents(
    const __spoor_runtime_FlushTraceEventsCallback completion) -> void {
  runtime_.Flush(completion);
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_ClearTraceEvents(
    const __spoor_runtime_FlushTraceEventsCallback completion) -> void {
  runtime_.Clear(completion);
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_FlushedTraceFiles(
    const __spoor_runtime_FlushedTraceEventsCallback completion) -> void {
  runtime_.FlushedTraceFiles([completion](const auto trace_file_paths) {
    auto** file_paths =
        static_cast<char**>(malloc(sizeof(char*) * trace_file_paths.size()));
    for (typename decltype(trace_file_paths)::size_type index{0};
         index < trace_file_paths.size(); ++index) {
      const auto& trace_file = trace_file_paths.at(index).string();
      file_paths[index] =
          static_cast<char*>(malloc(sizeof(char) * (trace_file.size() + 1)));
      trace_file.copy(file_paths[index], trace_file.size());
      file_paths[index][trace_file.size()] = '\0';
    };
    completion(__spoor_runtime_TraceFiles{
        .size = gsl::narrow_cast<int32>(trace_file_paths.size()),
        .file_paths = file_paths});
  });
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto __spoor_runtime_DeleteFlushedTraceFilesOlderThan(
    const __spoor_runtime_SystemTimestampSeconds system_timestamp_seconds,
    const __spoor_runtime_DeleteFlushedTraceFilesCallback completion) -> void {
  using DeletedFilesInfo =
      spoor::runtime::runtime_manager::RuntimeManager::DeletedFilesInfo;
  const auto system_timestamp =
      std::chrono::time_point<std::chrono::system_clock>{
          std::chrono::seconds{system_timestamp_seconds}};
  runtime_.DeleteFlushedTraceFilesOlderThan(
      system_timestamp,
      [completion](const DeletedFilesInfo deleted_files_info) {
        completion(__spoor_runtime_DeletedFilesInfo{
            .deleted_files = deleted_files_info.deleted_files,
            .deleted_bytes = deleted_files_info.deleted_bytes});
      });
}
