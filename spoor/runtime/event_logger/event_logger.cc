// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/event_logger/event_logger.h"

#include <chrono>
#include <utility>

#include "gsl/gsl"
#include "spoor/runtime/trace/trace.h"
#include "util/memory/owned_ptr.h"

namespace spoor::runtime::event_logger {

EventLogger::EventLogger(const Options options,
                         EventLoggerNotifier* event_logger_notifier)
    : options_{options},
      event_logger_notifier_{event_logger_notifier},
      pool_{nullptr} {
  if (event_logger_notifier_ != nullptr) {
    event_logger_notifier_->Subscribe(this);
  }
}

EventLogger::~EventLogger() {
  Flush();
  if (event_logger_notifier_ != nullptr) {
    event_logger_notifier_->Unsubscribe(this);
  }
}

auto EventLogger::SetEventLoggerNotifier(
    EventLoggerNotifier* event_logger_notifier) -> void {
  event_logger_notifier_ = event_logger_notifier;
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

auto EventLogger::LogEvent(const trace::Event event) -> void {
  if (pool_ == nullptr || !buffer_.has_value()) return;
  buffer_->Push(event);
  if (options_.flush_buffer_when_full && buffer_->Full()) Flush();
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
