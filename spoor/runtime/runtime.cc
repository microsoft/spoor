// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <span>
#include <system_error>
#include <utility>

#include "absl/strings/str_format.h"
#include "gsl/gsl"
#include "spoor/runtime/config/config.h"
#include "spoor/runtime/flush_queue/disk_flush_queue.h"
#include "spoor/runtime/runtime_manager/runtime_manager.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_file_reader.h"
#include "spoor/runtime/trace/trace_file_writer.h"
#include "util/file_system/local_file_system.h"
#include "util/numeric.h"
#include "util/time/clock.h"

namespace {

using spoor::runtime::runtime_manager::RuntimeManager;

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
const auto kConfig = spoor::runtime::config::Config::FromEnv();
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
util::time::SystemClock system_clock_{};
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
util::time::SteadyClock steady_clock_{};
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
util::file_system::LocalFileSystem file_system_{};
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
spoor::runtime::trace::TraceFileReader trace_reader_{
    {.file_system = &file_system_}};
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
spoor::runtime::trace::TraceFileWriter trace_writer_{};
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
spoor::runtime::flush_queue::DiskFlushQueue flush_queue_{
    {.trace_file_path = kConfig.trace_file_path,
     .buffer_retention_duration =
         std::chrono::nanoseconds{
             kConfig.event_buffer_retention_duration_nanoseconds},
     .system_clock = &system_clock_,
     .steady_clock = &steady_clock_,
     .trace_writer = &trace_writer_,
     .session_id = kConfig.session_id,
     .process_id = ::getpid(),
     .max_buffer_flush_attempts = kConfig.max_flush_buffer_to_file_attempts,
     .flush_all_events = kConfig.flush_all_events}};
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
RuntimeManager runtime_{
    {.steady_clock = &steady_clock_,
     .flush_queue = &flush_queue_,
     .thread_event_buffer_capacity = kConfig.thread_event_buffer_capacity,
     .reserved_pool_capacity = kConfig.reserved_event_pool_capacity,
     .reserved_pool_max_slice_capacity =
         kConfig.max_reserved_event_buffer_slice_capacity,
     .dynamic_pool_capacity = kConfig.dynamic_event_pool_capacity,
     .dynamic_pool_max_slice_capacity =
         kConfig.max_dynamic_event_buffer_slice_capacity,
     .dynamic_pool_borrow_cas_attempts =
         kConfig.dynamic_event_slice_borrow_cas_attempts,
     .max_buffer_flush_attempts = kConfig.max_flush_buffer_to_file_attempts,
     .flush_all_events = kConfig.flush_all_events}};

}  // namespace

auto _spoor_runtime_InitializeRuntime() -> void { runtime_.Initialize(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_DeinitializeRuntime() -> void { runtime_.Deinitialize(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_RuntimeInitialized() -> bool {
  return runtime_.Initialized();
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_EnableRuntime() -> void { runtime_.Enable(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_DisableRuntime() -> void { runtime_.Disable(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_RuntimeEnabled() -> bool { return runtime_.Enabled(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_LogFunctionEntry(
    const _spoor_runtime_FunctionId function_id) -> void {
  runtime_.LogEvent(spoor::runtime::trace::Event::Type::kFunctionEntry,
                    function_id);
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_LogFunctionExit(const _spoor_runtime_FunctionId function_id)
    -> void {
  runtime_.LogEvent(spoor::runtime::trace::Event::Type::kFunctionExit,
                    function_id);
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_FlushTraceEvents(
    const _spoor_runtime_FlushTraceEventsCallback callback) -> void {
  runtime_.Flush(callback);
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_ClearTraceEvents() -> void { runtime_.Clear(); }

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_FlushedTraceFiles(
    const _spoor_runtime_FlushedTraceFilesCallback callback) -> void {
  const auto callback_adapter =
      [callback](const std::vector<std::filesystem::path>& trace_file_paths) {
        if (callback == nullptr) return;
        std::span<char*> file_paths{
            static_cast<char**>(
                // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
                malloc(sizeof(char*) * trace_file_paths.size())),
            trace_file_paths.size()};
        for (std::vector<std::filesystem::path>::size_type index{0};
             index < trace_file_paths.size(); ++index) {
          const auto& trace_file = trace_file_paths.at(index).string();
          // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
          file_paths[index] = static_cast<char*>(
              // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
              malloc(sizeof(char) * (trace_file.size() + 1)));
          trace_file.copy(file_paths[index], trace_file.size());
        };
        callback(_spoor_runtime_TraceFiles{
            .size = gsl::narrow_cast<int32>(file_paths.size()),
            .file_paths = file_paths.data()});
      };  // NOLINT(clang-analyzer-unix.Malloc)
  std::error_code error{};
  const std::filesystem::directory_iterator directory{kConfig.trace_file_path,
                                                      error};
  if (error) {
    callback_adapter({});
    return;
  }
  RuntimeManager::FlushedTraceFiles(std::filesystem::begin(directory),
                                    std::filesystem::end(directory),
                                    &trace_reader_, callback_adapter);
}

void _spoor_runtime_ReleaseTraceFilePaths(
    _spoor_runtime_TraceFiles* trace_files) {
  std::span file_paths{trace_files->file_paths,
                       static_cast<std::size_t>(trace_files->size)};
  for (auto* path : file_paths) {
    // clang-format off NOLINTNEXTLINE(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory) clang-format on
    free(path);
  }
  // clang-format off NOLINTNEXTLINE(cppcoreguidelines-no-malloc, cppcoreguidelines-owning-memory) clang-format on
  free(trace_files->file_paths);
  trace_files->file_paths = nullptr;
  trace_files->size = 0;
}

// NOLINTNEXTLINE(readability-identifier-naming)
auto _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
    const _spoor_runtime_SystemTimestampSeconds system_timestamp_seconds,
    const _spoor_runtime_DeleteFlushedTraceFilesCallback callback) -> void {
  const auto callback_adapter =
      [callback](const RuntimeManager::DeletedFilesInfo deleted_files_info) {
        if (callback == nullptr) return;
        callback(_spoor_runtime_DeletedFilesInfo{
            .deleted_files = deleted_files_info.deleted_files,
            .deleted_bytes = deleted_files_info.deleted_bytes});
      };

  std::error_code error_code{};
  const std::filesystem::directory_iterator directory{kConfig.trace_file_path,
                                                      error_code};
  const auto error = static_cast<bool>(error_code);
  if (error) {
    callback_adapter({.deleted_files = 0, .deleted_bytes = 0});
    return;
  }
  const auto system_timestamp =
      std::chrono::time_point<std::chrono::system_clock>{
          std::chrono::seconds{system_timestamp_seconds}};
  RuntimeManager::DeleteFlushedTraceFilesOlderThan(
      system_timestamp, std::filesystem::begin(directory),
      std::filesystem::end(directory), &file_system_, &trace_reader_,
      callback_adapter);
}

auto _spoor_runtime_GetConfig() -> _spoor_runtime_Config {
  return {.trace_file_path = kConfig.trace_file_path.c_str(),
          .session_id = kConfig.session_id,
          .thread_event_buffer_capacity = kConfig.thread_event_buffer_capacity,
          .max_reserved_event_buffer_slice_capacity =
              kConfig.max_reserved_event_buffer_slice_capacity,
          .max_dynamic_event_buffer_slice_capacity =
              kConfig.max_dynamic_event_buffer_slice_capacity,
          .reserved_event_pool_capacity = kConfig.reserved_event_pool_capacity,
          .dynamic_event_pool_capacity = kConfig.dynamic_event_pool_capacity,
          .dynamic_event_slice_borrow_cas_attempts =
              kConfig.dynamic_event_slice_borrow_cas_attempts,
          .event_buffer_retention_duration_nanoseconds =
              kConfig.event_buffer_retention_duration_nanoseconds,
          .max_flush_buffer_to_file_attempts =
              kConfig.max_flush_buffer_to_file_attempts,
          .flush_all_events = kConfig.flush_all_events};
}
