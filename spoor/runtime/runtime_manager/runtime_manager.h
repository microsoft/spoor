// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <atomic>
#include <chrono>
#include <filesystem>
#include <functional>
#include <shared_mutex>
#include <thread>
#include <unordered_set>

#include "gsl/gsl"
#include "spoor/runtime/buffer/combined_buffer_slice_pool.h"
#include "spoor/runtime/event_logger/event_logger.h"
#include "spoor/runtime/event_logger/event_logger_notifier.h"
#include "spoor/runtime/flush_queue/flush_queue.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_reader.h"
#include "util/file_system/file_system.h"
#include "util/time/clock.h"

namespace spoor::runtime::runtime_manager {

class RuntimeManager final : public event_logger::EventLoggerNotifier {
 public:
  using Pool = buffer::CombinedBufferSlicePool<trace::Event>;
  using Buffer = buffer::CircularSliceBuffer<trace::Event>;
  using SizeType = Buffer::SizeType;

  struct alignas(128) Options {
    gsl::not_null<util::time::SteadyClock*> steady_clock;
    gsl::not_null<flush_queue::FlushQueue<Buffer>*> flush_queue;
    SizeType thread_event_buffer_capacity;
    SizeType reserved_pool_capacity;
    SizeType reserved_pool_max_slice_capacity;
    SizeType dynamic_pool_capacity;
    SizeType dynamic_pool_max_slice_capacity;
    SizeType dynamic_pool_borrow_cas_attempts;
    int32 max_buffer_flush_attempts;
    bool flush_all_events;
  };

  struct alignas(16) DeletedFilesInfo {
    int32 deleted_files;
    int64 deleted_bytes;
  };

  RuntimeManager() = delete;
  explicit RuntimeManager(Options options);
  RuntimeManager(const RuntimeManager&) = delete;
  RuntimeManager(RuntimeManager&&) noexcept = delete;
  auto operator=(const RuntimeManager&) = delete;
  auto operator=(RuntimeManager&&) noexcept = delete;
  ~RuntimeManager() override;

  auto Initialize() -> void;
  auto Deinitialize() -> void;
  auto Enable() -> void;
  auto Disable() -> void;
  auto Subscribe(gsl::not_null<event_logger::EventLogger*> subscriber)
      -> void override;
  auto Unsubscribe(gsl::not_null<event_logger::EventLogger*> subscriber)
      -> void override;

  auto LogEvent(trace::Event event) -> void;
  auto LogEvent(trace::EventType type,
                trace::TimestampNanoseconds steady_clock_timestamp,
                uint64 payload_1, uint32 payload_2) -> void;
  auto LogEvent(trace::EventType type, uint64 payload_1, uint32 payload_2)
      -> void;
  auto LogFunctionEntry(trace::FunctionId function_id) -> void;
  auto LogFunctionExit(trace::FunctionId function_id) -> void;

  auto Flush(std::function<void()> completion) -> void;
  auto Clear() -> void;

  [[nodiscard]] auto Initialized() const -> bool;
  [[nodiscard]] auto Enabled() const -> bool;

  template <class ConstIterator>
  static auto FlushedTraceFiles(
      ConstIterator directory_begin, ConstIterator directory_end,
      gsl::not_null<trace::TraceReader*> trace_reader,
      std::function<void(std::vector<std::filesystem::path>)> completion)
      -> void;
  template <class ConstIterator>
  static auto DeleteFlushedTraceFilesOlderThan(
      std::chrono::time_point<std::chrono::system_clock> timestamp,
      ConstIterator directory_begin, ConstIterator directory_end,
      gsl::not_null<util::file_system::FileSystem*> file_system,
      gsl::not_null<trace::TraceReader*> trace_reader,
      std::function<void(DeletedFilesInfo)> completion) -> void;

 private:
  Options options_;
  // Protects `pool_`, `event_loggers_`, `initialized_`, and `enabled_`.
  mutable std::shared_mutex lock_{};
  std::unique_ptr<Pool> pool_;
  std::unordered_set<event_logger::EventLogger*> event_loggers_;
  bool initialized_;

  // During pre-main stage, when LogEvent functions get called before
  // Spoor is finishing the default initialization, it will lead
  // to a crash when accessing some variables. By giving atomic_bool, with its
  // zero value, those two functions can bypass this crash. The atomic will make
  // sure it's thread-safe after the pre-main stage.
  std::atomic_bool enabled_;
};

template <class ConstIterator>
auto RuntimeManager::FlushedTraceFiles(
    ConstIterator directory_begin, ConstIterator directory_end,
    gsl::not_null<trace::TraceReader*> trace_reader,
    std::function<void(std::vector<std::filesystem::path>)> completion)
    -> void {
  std::thread{[directory_begin, directory_end, trace_reader,
               completion{std::move(completion)}] {
    std::vector<std::filesystem::path> trace_file_paths{};
    for (auto file{directory_begin}; file != directory_end; ++file) {
      if (trace_reader->MatchesTraceFileConvention(file->path())) {
        trace_file_paths.emplace_back(file->path());
      }
    }
    if (completion != nullptr) completion(std::move(trace_file_paths));
  }}.detach();
}

template <class ConstIterator>
auto RuntimeManager::DeleteFlushedTraceFilesOlderThan(
    const std::chrono::time_point<std::chrono::system_clock> timestamp,
    ConstIterator directory_begin, ConstIterator directory_end,
    gsl::not_null<util::file_system::FileSystem*> file_system,
    gsl::not_null<trace::TraceReader*> trace_reader,
    std::function<void(DeletedFilesInfo)> completion) -> void {
  std::thread{[directory_begin, directory_end, file_system, trace_reader,
               timestamp, completion{std::move(completion)}] {
    DeletedFilesInfo deleted_files_info{.deleted_files = 0, .deleted_bytes = 0};
    for (auto file{directory_begin}; file != directory_end; ++file) {
      if (!trace_reader->MatchesTraceFileConvention(file->path())) {
        continue;
      }
      const auto result = trace_reader->ReadHeader(file->path());
      if (result.IsErr()) continue;
      const auto header = result.Ok();
      const auto header_system_clock_timestamp =
          std::chrono::time_point<std::chrono::system_clock>{
              std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::nanoseconds{header.system_clock_timestamp})};
      if (timestamp < header_system_clock_timestamp) continue;
      const auto file_size_result = file_system->FileSize(file->path());
      const auto success_result = file_system->Remove(file->path());
      if (success_result.IsOk()) {
        ++deleted_files_info.deleted_files;
        deleted_files_info.deleted_bytes += file_size_result.OkOr(0);
      }
    }
    if (completion != nullptr) completion(deleted_files_info);
  }}.detach();
}

auto operator==(const RuntimeManager::DeletedFilesInfo& lhs,
                const RuntimeManager::DeletedFilesInfo& rhs) -> bool;

}  // namespace spoor::runtime::runtime_manager
