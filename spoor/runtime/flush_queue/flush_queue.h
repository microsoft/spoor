#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>

#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_writer.h"
#include "util/numeric.h"
#include "util/time/clock.h"

namespace spoor::runtime::flush_queue {

class FlushQueue {
 public:
  using SizeType = std::size_t;
  using Buffer = spoor::runtime::buffer::CircularSliceBuffer<trace::Event>;

  struct Options {
    std::filesystem::path trace_file_path;
    std::chrono::nanoseconds buffer_retention_duration;
    util::time::SystemClock* system_clock;
    util::time::SteadyClock* steady_clock;
    trace::TraceWriter* trace_writer;
    trace::SessionId session_id;
    trace::ProcessId process_id;
    int32 max_buffer_flush_attempts;
    bool flush_immediately;
  };

  FlushQueue() = delete;
  explicit FlushQueue(Options options);
  FlushQueue(const FlushQueue&) = delete;
  FlushQueue(FlushQueue&&) noexcept = delete;
  auto operator=(const FlushQueue&) -> FlushQueue& = delete;
  auto operator=(FlushQueue&&) noexcept -> FlushQueue& = delete;
  ~FlushQueue();

  auto Run() -> void;
  // This is a synchronous operation that waits for the queue to empty. Buffers
  // will be retained until the deadline or until the queue is flushed or
  // cleared.
  auto DrainAndStop() -> void;
  auto Enqueue(Buffer&& buffer) -> void;
  auto Flush() -> void;
  auto Clear() -> void;

  [[nodiscard]] auto Size() const -> SizeType;
  [[nodiscard]] auto Empty() const -> bool;

 private:
  struct FlushInfo {
    Buffer buffer;
    std::chrono::time_point<std::chrono::steady_clock> flush_timestamp;
    trace::ThreadId thread_id;
    int32 remaining_flush_attempts{};
  };

  Options options_;
  std::thread flush_thread_;
  mutable std::shared_mutex lock_;  // Guards `queue_` and `flush_timestamp_`.
  std::queue<FlushInfo> queue_;
  std::chrono::time_point<std::chrono::steady_clock> flush_timestamp_;
  std::atomic_size_t queue_size_;
  std::atomic_bool running_;
  std::atomic_bool draining_;

  auto TraceFilePath(const FlushInfo& flush_info) const
      -> std::filesystem::path;
  auto TraceFileHeader(const FlushInfo& flush_info) const -> trace::Header;
};

}  // namespace spoor::runtime::flush_queue
