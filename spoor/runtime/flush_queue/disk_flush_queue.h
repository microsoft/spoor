#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <deque>
#include <filesystem>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <unordered_set>

#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/flush_queue/flush_queue.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_writer.h"
#include "util/numeric.h"
#include "util/time/clock.h"

namespace spoor::runtime::flush_queue {

class DiskFlushQueue
    : public FlushQueue<buffer::CircularSliceBuffer<trace::Event>> {
 public:
  using Buffer = buffer::CircularSliceBuffer<trace::Event>;
  using SizeType = Buffer::SizeType;

  struct Options {
    std::filesystem::path trace_file_path;
    std::chrono::nanoseconds buffer_retention_duration;
    util::time::SystemClock* system_clock;
    util::time::SteadyClock* steady_clock;
    trace::TraceWriter* trace_writer;
    trace::SessionId session_id;
    trace::ProcessId process_id;
    int32 max_buffer_flush_attempts;
    bool flush_all_events;
  };

  DiskFlushQueue() = delete;
  explicit DiskFlushQueue(Options options);
  DiskFlushQueue(const DiskFlushQueue&) = delete;
  DiskFlushQueue(DiskFlushQueue&&) noexcept = delete;
  auto operator=(const DiskFlushQueue&) -> DiskFlushQueue& = delete;
  auto operator=(DiskFlushQueue&&) noexcept -> DiskFlushQueue& = delete;
  ~DiskFlushQueue();

  auto Run() -> void override;
  // Waits for the queue to empty synchronously. Buffers will be retained until
  // the deadline or until the queue is flushed or cleared.
  auto Enqueue(Buffer&& buffer) -> void override;
  auto DrainAndStop() -> void override;
  auto Flush(std::function<void()> completion) -> void override;
  auto Clear() -> void override;

  [[nodiscard]] auto Size() const -> SizeType;
  [[nodiscard]] auto Empty() const -> bool;

 private:
  struct FlushInfo {
    Buffer buffer;
    std::chrono::time_point<std::chrono::steady_clock> flush_timestamp;
    trace::ThreadId thread_id;
    int64 id;
    int32 remaining_flush_attempts{};
  };

  Options options_;
  std::thread flush_thread_;
  // Guards `queue_`, `flush_completion_`, `manual_flush_record_ids_`, and
  // `flush_timestamp_`, and `next_flush_info_id_`.
  mutable std::shared_mutex lock_;
  std::deque<FlushInfo> queue_;
  std::optional<std::function<void()>> flush_completion_;
  std::unordered_set<int64> manual_flush_record_ids_;
  std::chrono::time_point<std::chrono::steady_clock> flush_timestamp_;
  int64 next_flush_info_id_;
  std::atomic_size_t queue_size_;
  std::atomic_bool running_;
  std::atomic_bool draining_;

  auto TraceFilePath(const FlushInfo& flush_info) const
      -> std::filesystem::path;
  auto TraceFileHeader(const FlushInfo& flush_info) const -> trace::Header;
};

}  // namespace spoor::runtime::flush_queue
