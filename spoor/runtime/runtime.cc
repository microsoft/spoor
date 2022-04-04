// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
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
#include "util/file_system/util.h"
#include "util/numeric.h"
#include "util/time/clock.h"

namespace spoor::runtime {

static_assert(std::is_same_v<DurationNanoseconds, trace::DurationNanoseconds>);
static_assert(std::is_same_v<EventType, trace::EventType>);
static_assert(std::is_same_v<FunctionId, trace::FunctionId>);
static_assert(std::is_same_v<SessionId, trace::SessionId>);
static_assert(
    std::is_same_v<TimestampNanoseconds, trace::TimestampNanoseconds>);
static_assert(std::is_same_v<FunctionId, _spoor_runtime_FunctionId>);

}  // namespace spoor::runtime

namespace {

class SteadyClock {
 public:
  ~SteadyClock() = default;
  SteadyClock(const SteadyClock&) = delete;
  SteadyClock(SteadyClock&&) = delete;
  auto operator=(const SteadyClock&) -> SteadyClock& = delete;
  auto operator=(SteadyClock&&) -> SteadyClock& = delete;

  static auto Instance() -> util::time::SteadyClock&;

 private:
  SteadyClock() = default;
};

class SystemClock {
 public:
  ~SystemClock() = default;
  SystemClock(const SystemClock&) = delete;
  SystemClock(SystemClock&&) = delete;
  auto operator=(const SystemClock&) -> SystemClock& = delete;
  auto operator=(SystemClock&&) -> SystemClock& = delete;

  static auto Instance() -> util::time::SystemClock&;

 private:
  SystemClock() = default;
};

class LocalFileSystem {
 public:
  ~LocalFileSystem() = default;
  LocalFileSystem(const LocalFileSystem&) = delete;
  LocalFileSystem(LocalFileSystem&&) = delete;
  auto operator=(const LocalFileSystem&) -> LocalFileSystem& = delete;
  auto operator=(LocalFileSystem&&) -> LocalFileSystem& = delete;

  static auto Instance() -> util::file_system::LocalFileSystem&;

 private:
  LocalFileSystem() = default;
};

class TraceFileReader {
 public:
  ~TraceFileReader() = default;
  TraceFileReader(const TraceFileReader&) = delete;
  TraceFileReader(TraceFileReader&&) = delete;
  auto operator=(const TraceFileReader&) -> TraceFileReader& = delete;
  auto operator=(TraceFileReader&&) -> TraceFileReader& = delete;

  static auto Instance() -> spoor::runtime::trace::TraceFileReader&;

 private:
  TraceFileReader() = default;
};

class TraceFileWriter {
 public:
  ~TraceFileWriter() = default;
  TraceFileWriter(const TraceFileWriter&) = delete;
  TraceFileWriter(TraceFileWriter&&) = delete;
  auto operator=(const TraceFileWriter&) -> TraceFileWriter& = delete;
  auto operator=(TraceFileWriter&&) -> TraceFileWriter& = delete;

  static auto Instance() -> spoor::runtime::trace::TraceFileWriter&;

 private:
  TraceFileWriter() = default;
};

class Config {
 public:
  ~Config() = default;
  Config(const Config&) = delete;
  Config(Config&&) = delete;
  auto operator=(const Config&) -> Config& = delete;
  auto operator=(Config&&) -> Config& = delete;

  static auto Instance() -> const spoor::runtime::config::Config&;

 private:
  Config() = default;
};

class RuntimeManager {
 public:
  ~RuntimeManager() = default;
  RuntimeManager(const RuntimeManager&) = delete;
  RuntimeManager(RuntimeManager&&) = delete;
  auto operator=(const RuntimeManager&) -> RuntimeManager& = delete;
  auto operator=(RuntimeManager&&) -> RuntimeManager& = delete;

  static auto Instance() -> spoor::runtime::runtime_manager::RuntimeManager&;
  template <class ConstIterator>
  static auto FlushedTraceFiles(
      ConstIterator directory_begin, ConstIterator directory_end,
      gsl::not_null<spoor::runtime::trace::TraceReader*> trace_reader,
      std::function<void(std::vector<std::filesystem::path>)> completion)
      -> void;
  template <class ConstIterator>
  static auto DeleteFlushedTraceFilesOlderThan(
      std::chrono::time_point<std::chrono::system_clock> timestamp,
      ConstIterator directory_begin, ConstIterator directory_end,
      gsl::not_null<util::file_system::FileSystem*> file_system,
      gsl::not_null<spoor::runtime::trace::TraceReader*> trace_reader,
      std::function<void(
          spoor::runtime::runtime_manager::RuntimeManager::DeletedFilesInfo)>
          completion) -> void;

 private:
  RuntimeManager() = default;
};

auto SystemClock::Instance() -> util::time::SystemClock& {
  static util::time::SystemClock system_clock{};
  return system_clock;
}

auto SteadyClock::Instance() -> util::time::SteadyClock& {
  static util::time::SteadyClock steady_clock{};
  return steady_clock;
}

auto LocalFileSystem::Instance() -> util::file_system::LocalFileSystem& {
  static util::file_system::LocalFileSystem file_system{};
  return file_system;
}

auto TraceFileReader::Instance() -> spoor::runtime::trace::TraceFileReader& {
  static spoor::runtime::trace::TraceFileReader trace_file_reader{{
      .file_system{std::make_unique<util::file_system::LocalFileSystem>()},
      .file_reader{std::make_unique<util::file_system::LocalFileReader>()},
  }};
  return trace_file_reader;
}

auto TraceFileWriter::Instance() -> spoor::runtime::trace::TraceFileWriter& {
  static spoor::runtime::trace::TraceFileWriter trace_file_writer{{
      .file_system{std::make_unique<util::file_system::LocalFileSystem>()},
      .file_writer{std::make_unique<util::file_system::LocalFileWriter>()},
      .compression_strategy = Config::Instance().compression_strategy,
      .initial_buffer_capacity =
          Config::Instance().thread_event_buffer_capacity,
      .directory = Config::Instance().trace_file_path,
      .create_directory = true,
  }};
  return trace_file_writer;
}

auto Config::Instance() -> const spoor::runtime::config::Config& {
  static const util::file_system::PathExpansionOptions path_expansion_options{
      .get_env = std::getenv,
      .expand_tilde = true,
      .expand_environment_variables = true,
  };
  static auto sources = [&] {
    std::vector<std::unique_ptr<spoor::runtime::config::Source>> sources{};
    spoor::runtime::config::EnvSource::Options env_source_options{
        .get_env = std::getenv};
    sources.emplace_back(std::make_unique<spoor::runtime::config::EnvSource>(
        std::move(env_source_options)));
    const auto config_file_path = [&]() -> std::optional<std::string> {
      auto path = spoor::runtime::ConfigFilePath();
      if (!path.has_value()) return {};
      return util::file_system::ExpandPath(path.value(),
                                           path_expansion_options);
    }();
    if (config_file_path.has_value()) {
      spoor::runtime::config::FileSource::Options file_source_options{
          .file_reader{std::make_unique<util::file_system::LocalFileReader>()},
          .file_path{config_file_path.value()},
          .get_env{std::getenv},
      };
      sources.emplace_back(std::make_unique<spoor::runtime::config::FileSource>(
          std::move(file_source_options)));
    }
    return sources;
  }();
  static auto instance = spoor::runtime::config::Config::FromSourcesOrDefault(
      std::move(sources), spoor::runtime::config::Config::Default(),
      path_expansion_options);
  return instance;
};

auto RuntimeManager::Instance()
    -> spoor::runtime::runtime_manager::RuntimeManager& {
  static spoor::runtime::flush_queue::DiskFlushQueue flush_queue{{
      .buffer_retention_duration =
          std::chrono::nanoseconds{
              Config::Instance().event_buffer_retention_duration_nanoseconds},
      .system_clock = &SystemClock::Instance(),
      .steady_clock = &SteadyClock::Instance(),
      .trace_writer = &TraceFileWriter::Instance(),
      .session_id = Config::Instance().session_id,
      .process_id = ::getpid(),
      .max_buffer_flush_attempts =
          Config::Instance().max_flush_buffer_to_file_attempts,
      .flush_all_events = Config::Instance().flush_all_events,
  }};
  static spoor::runtime::runtime_manager::RuntimeManager runtime_manager{{
      .steady_clock = &SteadyClock::Instance(),
      .flush_queue = &flush_queue,
      .thread_event_buffer_capacity =
          Config::Instance().thread_event_buffer_capacity,
      .reserved_pool_capacity = Config::Instance().reserved_event_pool_capacity,
      .reserved_pool_max_slice_capacity =
          Config::Instance().max_reserved_event_buffer_slice_capacity,
      .dynamic_pool_capacity = Config::Instance().dynamic_event_pool_capacity,
      .dynamic_pool_max_slice_capacity =
          Config::Instance().max_dynamic_event_buffer_slice_capacity,
      .dynamic_pool_borrow_cas_attempts =
          Config::Instance().dynamic_event_slice_borrow_cas_attempts,
      .max_buffer_flush_attempts =
          Config::Instance().max_flush_buffer_to_file_attempts,
      .flush_all_events = Config::Instance().flush_all_events,
  }};
  return runtime_manager;
}

template <class ConstIterator>
auto RuntimeManager::FlushedTraceFiles(
    ConstIterator directory_begin, ConstIterator directory_end,
    gsl::not_null<spoor::runtime::trace::TraceReader*> trace_reader,
    std::function<void(std::vector<std::filesystem::path>)> completion)
    -> void {
  spoor::runtime::runtime_manager::RuntimeManager::FlushedTraceFiles(
      directory_begin, directory_end, trace_reader, completion);
}

template <class ConstIterator>
auto RuntimeManager::DeleteFlushedTraceFilesOlderThan(
    std::chrono::time_point<std::chrono::system_clock> timestamp,
    ConstIterator directory_begin, ConstIterator directory_end,
    gsl::not_null<util::file_system::FileSystem*> file_system,
    gsl::not_null<spoor::runtime::trace::TraceReader*> trace_reader,
    std::function<
        void(spoor::runtime::runtime_manager::RuntimeManager::DeletedFilesInfo)>
        completion) -> void {
  spoor::runtime::runtime_manager::RuntimeManager::
      DeleteFlushedTraceFilesOlderThan(timestamp, directory_begin,
                                       directory_end, file_system, trace_reader,
                                       completion);
}

}  // namespace

namespace spoor::runtime {

auto Initialize() -> void { RuntimeManager::Instance().Initialize(); }

auto Deinitialize() -> void { RuntimeManager::Instance().Deinitialize(); }

auto Initialized() -> bool { return RuntimeManager::Instance().Initialized(); }

auto Enable() -> void { RuntimeManager::Instance().Enable(); }

auto Disable() -> void { RuntimeManager::Instance().Disable(); }

auto Enabled() -> bool { return RuntimeManager::Instance().Enabled(); }

auto LogEvent(const EventType event,
              const TimestampNanoseconds steady_clock_timestamp,
              const std::uint64_t payload_1, const std::uint64_t payload_2)
    -> void {
  RuntimeManager::Instance().LogEvent(event, steady_clock_timestamp, payload_1,
                                      payload_2);
}

auto LogEvent(const EventType event, const std::uint64_t payload_1,
              const std::uint64_t payload_2) -> void {
  RuntimeManager::Instance().LogEvent(event, payload_1, payload_2);
}

auto FlushTraceEvents(std::function<void()> callback) -> void {
  RuntimeManager::Instance().Flush(std::move(callback));
}

auto ClearTraceEvents() -> void { RuntimeManager::Instance().Clear(); }

auto FlushedTraceFiles(
    std::function<void(std::vector<std::filesystem::path>)> callback) -> void {
  std::error_code error{};
  const std::filesystem::directory_iterator directory{
      ::Config::Instance().trace_file_path, error};
  if (error) {
    if (callback != nullptr) {
      std::thread{std::move(callback), std::vector<std::filesystem::path>{}}
          .detach();
    }
    return;
  }
  RuntimeManager::FlushedTraceFiles(
      std::filesystem::begin(directory), std::filesystem::end(directory),
      &TraceFileReader::Instance(), std::move(callback));
}

auto DeleteFlushedTraceFilesOlderThan(
    const SystemTimestampSeconds system_timestamp_seconds,
    std::function<void(DeletedFilesInfo)> callback) -> void {
  auto callback_adapter =
      [callback{std::move(callback)}](
          const spoor::runtime::runtime_manager::RuntimeManager::
              DeletedFilesInfo deleted_files_info) {
        if (callback == nullptr) return;
        callback({.deleted_files = deleted_files_info.deleted_files,
                  .deleted_bytes = deleted_files_info.deleted_bytes});
      };

  std::error_code error{};
  const std::filesystem::directory_iterator directory{
      ::Config::Instance().trace_file_path, error};
  if (error) {
    std::thread{
        std::move(callback_adapter),
        spoor::runtime::runtime_manager::RuntimeManager::DeletedFilesInfo{
            .deleted_files = 0, .deleted_bytes = 0}}
        .detach();
    return;
  }
  const auto system_timestamp =
      std::chrono::time_point<std::chrono::system_clock>{
          std::chrono::seconds{system_timestamp_seconds}};
  RuntimeManager::DeleteFlushedTraceFilesOlderThan(
      system_timestamp, std::filesystem::begin(directory),
      std::filesystem::end(directory), &LocalFileSystem::Instance(),
      &TraceFileReader::Instance(), std::move(callback_adapter));
}

auto GetConfig() -> spoor::runtime::Config {
  return {
      .trace_file_path = ::Config::Instance().trace_file_path,
      .session_id = ::Config::Instance().session_id,
      .thread_event_buffer_capacity =
          ::Config::Instance().thread_event_buffer_capacity,
      .max_reserved_event_buffer_slice_capacity =
          ::Config::Instance().max_reserved_event_buffer_slice_capacity,
      .max_dynamic_event_buffer_slice_capacity =
          ::Config::Instance().max_dynamic_event_buffer_slice_capacity,
      .reserved_event_pool_capacity =
          ::Config::Instance().reserved_event_pool_capacity,
      .dynamic_event_pool_capacity =
          ::Config::Instance().dynamic_event_pool_capacity,
      .dynamic_event_slice_borrow_cas_attempts =
          ::Config::Instance().dynamic_event_slice_borrow_cas_attempts,
      .event_buffer_retention_duration_nanoseconds =
          ::Config::Instance().event_buffer_retention_duration_nanoseconds,
      .max_flush_buffer_to_file_attempts =
          ::Config::Instance().max_flush_buffer_to_file_attempts,
      .flush_all_events = ::Config::Instance().flush_all_events,
  };
}

auto StubImplementation() -> bool { return false; }

}  // namespace spoor::runtime

auto _spoor_runtime_LogFunctionEntry(
    const _spoor_runtime_FunctionId function_id) -> void {
  RuntimeManager::Instance().LogFunctionEntry(function_id);
}

auto _spoor_runtime_LogFunctionExit(const _spoor_runtime_FunctionId function_id)
    -> void {
  RuntimeManager::Instance().LogFunctionExit(function_id);
}
