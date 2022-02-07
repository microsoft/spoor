// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <system_error>
#include <thread>
#include <type_traits>
#include <utility>

#include "absl/strings/str_format.h"
#include "gsl/gsl"
#include "spoor/runtime/config/config.h"
#include "spoor/runtime/config/env_source.h"
#include "spoor/runtime/config/file_source.h"
#include "spoor/runtime/config/source.h"
#include "spoor/runtime/flush_queue/disk_flush_queue.h"
#include "spoor/runtime/runtime_manager/runtime_manager.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_file_reader.h"
#include "spoor/runtime/trace/trace_file_writer.h"
#include "util/compression/compressor_factory.h"
#include "util/file_system/local_file_reader.h"
#include "util/file_system/local_file_system.h"
#include "util/file_system/local_file_writer.h"
#include "util/numeric.h"
#include "util/time/clock.h"

namespace spoor::runtime {

static_assert(std::is_same_v<DurationNanoseconds, trace::DurationNanoseconds>);
static_assert(std::is_same_v<EventType, trace::EventType>);
static_assert(std::is_same_v<FunctionId, trace::FunctionId>);
static_assert(std::is_same_v<SessionId, trace::SessionId>);
static_assert(
    std::is_same_v<TimestampNanoseconds, trace::TimestampNanoseconds>);

}  // namespace spoor::runtime

namespace {

using spoor::runtime::runtime_manager::RuntimeManager;

const auto kConfig = [] {  // NOLINT(fuchsia-statically-constructed-objects)
  spoor::runtime::config::EnvSource::Options env_source_options{
      .get_env = std::getenv};
  spoor::runtime::config::FileSource::Options file_source_options{
      .file_reader{std::make_unique<util::file_system::LocalFileReader>()},
      .file_path{"spoor_runtime_config.toml"},
  };
  auto sources = std::vector<std::unique_ptr<spoor::runtime::config::Source>>{};
  sources.reserve(2);
  sources.emplace_back(std::make_unique<spoor::runtime::config::EnvSource>(
      std::move(env_source_options)));
  sources.emplace_back(std::make_unique<spoor::runtime::config::FileSource>(
      std::move(file_source_options)));
  return spoor::runtime::config::Config::FromSourcesOrDefault(
      std::move(sources), spoor::runtime::config::Config::Default());
}();
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
util::time::SystemClock system_clock_{};
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
util::time::SteadyClock steady_clock_{};
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
util::file_system::LocalFileSystem file_system_{};
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
spoor::runtime::trace::TraceFileReader trace_reader_{{
    .file_system{std::make_unique<util::file_system::LocalFileSystem>()},
    .file_reader{std::make_unique<util::file_system::LocalFileReader>()},
}};
// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
spoor::runtime::trace::TraceFileWriter trace_writer_{{
    .file_writer{std::make_unique<util::file_system::LocalFileWriter>()},
    .compression_strategy = kConfig.compression_strategy,
    .initial_buffer_capacity = kConfig.thread_event_buffer_capacity,
}};
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

namespace spoor::runtime {

auto Initialize() -> void { runtime_.Initialize(); }

auto Deinitialize() -> void { runtime_.Deinitialize(); }

auto Initialized() -> bool { return runtime_.Initialized(); }

auto Enable() -> void { runtime_.Enable(); }

auto Disable() -> void { runtime_.Disable(); }

auto Enabled() -> bool { return runtime_.Enabled(); }

auto LogEvent(const EventType event,
              const TimestampNanoseconds steady_clock_timestamp,
              const std::uint64_t payload_1, const std::uint64_t payload_2)
    -> void {
  runtime_.LogEvent(event, steady_clock_timestamp, payload_1, payload_2);
}

auto LogEvent(const EventType event, const std::uint64_t payload_1,
              const std::uint64_t payload_2) -> void {
  runtime_.LogEvent(event, payload_1, payload_2);
}

auto FlushTraceEvents(std::function<void()> callback) -> void {
  runtime_.Flush(std::move(callback));
}

auto ClearTraceEvents() -> void { runtime_.Clear(); }

auto FlushedTraceFiles(
    std::function<void(std::vector<std::filesystem::path>)> callback) -> void {
  std::error_code error{};
  const std::filesystem::directory_iterator directory{kConfig.trace_file_path,
                                                      error};
  if (error) {
    if (callback != nullptr) {
      std::thread{std::move(callback), std::vector<std::filesystem::path>{}}
          .detach();
    }
    return;
  }
  RuntimeManager::FlushedTraceFiles(std::filesystem::begin(directory),
                                    std::filesystem::end(directory),
                                    &trace_reader_, std::move(callback));
}

auto DeleteFlushedTraceFilesOlderThan(
    const SystemTimestampSeconds system_timestamp_seconds,
    std::function<void(DeletedFilesInfo)> callback) -> void {
  auto callback_adapter =
      [callback{std::move(callback)}](
          const RuntimeManager::DeletedFilesInfo deleted_files_info) {
        if (callback == nullptr) return;
        callback({.deleted_files = deleted_files_info.deleted_files,
                  .deleted_bytes = deleted_files_info.deleted_bytes});
      };

  std::error_code error{};
  const std::filesystem::directory_iterator directory{kConfig.trace_file_path,
                                                      error};
  if (error) {
    std::thread{std::move(callback_adapter),
                RuntimeManager::DeletedFilesInfo{.deleted_files = 0,
                                                 .deleted_bytes = 0}}
        .detach();
    return;
  }
  const auto system_timestamp =
      std::chrono::time_point<std::chrono::system_clock>{
          std::chrono::seconds{system_timestamp_seconds}};
  RuntimeManager::DeleteFlushedTraceFilesOlderThan(
      system_timestamp, std::filesystem::begin(directory),
      std::filesystem::end(directory), &file_system_, &trace_reader_,
      std::move(callback_adapter));
}

auto GetConfig() -> Config {
  return {.trace_file_path = kConfig.trace_file_path,
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

auto StubImplementation() -> bool { return false; }

}  // namespace spoor::runtime

auto _spoor_runtime_LogFunctionEntry(
    const spoor::runtime::FunctionId function_id) -> void {
  runtime_.LogFunctionEntry(function_id);
}

auto _spoor_runtime_LogFunctionExit(
    const spoor::runtime::FunctionId function_id) -> void {
  runtime_.LogFunctionExit(function_id);
}
