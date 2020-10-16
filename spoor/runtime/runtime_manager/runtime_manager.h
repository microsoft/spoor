#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <shared_mutex>
#include <unordered_set>

#include "gsl/gsl"
#include "spoor/runtime/buffer/amalgamated_buffer_slice_pool.h"
#include "spoor/runtime/event_logger/event_logger.h"
#include "spoor/runtime/event_logger/event_logger_notifier.h"
#include "spoor/runtime/flush_queue/flush_queue.h"
#include "spoor/runtime/trace/trace.h"

namespace spoor::runtime::runtime_manager {

class RuntimeManager final : public event_logger::EventLoggerNotifier {
 public:
  using Pool = buffer::AmalgamatedBufferSlicePool<trace::Event>;
  using Buffer = buffer::CircularSliceBuffer<trace::Event>;
  using SizeType = Buffer::SizeType;

  struct Options {
    std::filesystem::path trace_file_path;
    util::time::SteadyClock* steady_clock;
    flush_queue::FlushQueue<Buffer>* flush_queue;
    SizeType thread_event_buffer_capacity;
    SizeType reserved_pool_capacity;
    SizeType reserved_pool_max_slice_capacity;
    SizeType dynamic_pool_capacity;
    SizeType dynamic_pool_max_slice_capacity;
    SizeType dynamic_pool_borrow_cas_attempts;
    int32 max_buffer_flush_attempts;
    bool flush_all_events;
  };

  struct DeletedFilesInfo {
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

  auto LogEvent(trace::Event::Type type, trace::FunctionId function_id) -> void;

  auto Flush(std::function<void(/*TODO*/)> completion) -> void;
  auto Clear(std::function<void(/*TODO*/)> completion) -> void;

  [[nodiscard]] auto Initialized() const -> bool;
  [[nodiscard]] auto Enabled() const -> bool;

  auto FlushedTraceFiles(
      std::function<void(std::vector<std::filesystem::path>)> completion) const
      -> void;
  auto DeleteFlushedTraceFilesOlderThan(
      std::chrono::time_point<std::chrono::system_clock> timestamp,
      std::function<void(DeletedFilesInfo)> completion) const -> void;

 private:
  Options options_;
  std::unique_ptr<Pool> pool_;
  // Protects `event_loggers_`, `initialized_`, and `enabled_`.
  mutable std::shared_mutex lock_{};
  std::unordered_set<event_logger::EventLogger*> event_loggers_;
  bool initialized_;
  bool enabled_;
};

}  // namespace spoor::runtime::runtime_manager
