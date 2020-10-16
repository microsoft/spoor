#pragma once

#include <optional>

#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/event_logger/event_logger_notifier.h"
#include "spoor/runtime/flush_queue/flush_queue.h"
#include "spoor/runtime/trace/trace.h"
#include "util/memory/owned_ptr.h"
#include "util/time/clock.h"

namespace spoor::runtime::event_logger {

class EventLogger {
 public:
  using Pool = buffer::BufferSlicePool<trace::Event>;
  using Buffer = buffer::CircularSliceBuffer<trace::Event>;
  using SizeType = Buffer::SizeType;

  struct Options {
    util::time::SteadyClock* steady_clock;
    EventLoggerNotifier* event_logger_notifier;
    flush_queue::FlushQueue<Buffer>* flush_queue;
    SizeType preferred_capacity;
    bool flush_buffer_when_full;
  };

  EventLogger() = delete;
  explicit EventLogger(Options options);
  EventLogger(const EventLogger&) = delete;
  EventLogger(EventLogger&&) noexcept = delete;
  auto operator=(const EventLogger&) = delete;
  auto operator=(EventLogger&&) noexcept = delete;
  ~EventLogger();

  auto SetPool(Pool* pool) -> void;
  auto LogEvent(trace::Event::Type type, trace::FunctionId function_id) -> void;

  auto Flush() -> void;
  auto Clear() -> void;

  [[nodiscard]] auto Size() const -> SizeType;
  [[nodiscard]] auto Capacity() const -> SizeType;
  [[nodiscard]] auto Empty() const -> bool;
  [[nodiscard]] auto Full() const -> bool;

 private:
  Options options_;
  Pool* pool_;
  std::optional<Buffer> buffer_;
};

}  // namespace spoor::runtime::event_logger
