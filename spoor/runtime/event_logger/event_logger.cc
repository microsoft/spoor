#include "spoor/runtime/event_logger/event_logger.h"

#include <chrono>
#include <utility>

#include "gsl/gsl"
#include "util/memory/owned_ptr.h"

namespace spoor::runtime::event_logger {

EventLogger::EventLogger(const Options options)
    : options_{options}, pool_{nullptr} {
  options_.event_logger_notifier->Subscribe(this);
}

EventLogger::~EventLogger() {
  Flush();
  options_.event_logger_notifier->Unsubscribe(this);
}

auto EventLogger::SetPool(Pool* pool) -> void {
  if (pool == nullptr) {
    Flush();
    buffer_ = {};
  } else {
    buffer_ = Buffer{
        {.buffer_slice_pool = pool, .capacity = options_.preferred_capacity}};
  }
  pool_ = pool;
}

auto EventLogger::LogEvent(trace::Event::Type type,
                           trace::FunctionId function_id) -> void {
  const auto now = options_.steady_clock->Now();
  if (pool_ == nullptr || !buffer_.has_value()) return;
  const auto now_nanoseconds =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          now.time_since_epoch())
          .count();
  const trace::Event event{type, function_id, now_nanoseconds};
  buffer_->Push(event);
  if (options_.flush_buffer_when_full && buffer_->WillWrapOnNextPush()) Flush();
}

auto EventLogger::Flush() -> void {
  if (!buffer_.has_value() || buffer_.value().Empty()) return;
  options_.flush_queue->Enqueue(std::move(buffer_.value()));
  buffer_ = Buffer{
      {.buffer_slice_pool = pool_, .capacity = options_.preferred_capacity}};
}

auto EventLogger::Clear() -> void {
  if (!buffer_.has_value()) return;
  buffer_->Clear();
}

auto EventLogger::Size() const -> SizeType {
  if (!buffer_.has_value()) return 0;
  return buffer_->Size();
}

auto EventLogger::Capacity() const -> SizeType {
  if (!buffer_.has_value()) return 0;
  return buffer_->Capacity();
}

auto EventLogger::Empty() const -> bool {
  if (!buffer_.has_value()) return true;
  return buffer_->Empty();
}

auto EventLogger::Full() const -> bool {
  if (!buffer_.has_value()) return true;
  return buffer_->Full();
}

}  // namespace spoor::runtime::event_logger
