// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime_manager/runtime_manager.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>

#include "gsl/gsl"
#include "spoor/runtime/trace/trace_reader.h"
#include "util/file_system/file_system.h"

namespace spoor::runtime::runtime_manager {

RuntimeManager::RuntimeManager(Options options)
    : options_{std::move(options)},
      pool_{nullptr},
      initialized_{false},
      enabled_{false} {}

RuntimeManager::~RuntimeManager() { Deinitialize(); }

auto RuntimeManager::Initialize() -> void {
  std::unique_lock lock{lock_};
  if (initialized_) return;
  const Pool::Options pool_options{
      .reserved_pool_options = {.max_slice_capacity =
                                    options_.reserved_pool_max_slice_capacity,
                                .capacity = options_.reserved_pool_capacity},
      .dynamic_pool_options = {
          .max_slice_capacity = options_.dynamic_pool_max_slice_capacity,
          .capacity = options_.dynamic_pool_capacity,
          .borrow_cas_attempts = options_.dynamic_pool_borrow_cas_attempts}};
  pool_ = std::make_unique<Pool>(pool_options);
  options_.flush_queue->Run();
  for (auto* event_logger : event_loggers_) {
    event_logger->SetPool(pool_.get());
  }

  initialized_ = true;
}

auto RuntimeManager::Deinitialize() -> void {
  Disable();
  {
    std::unique_lock lock{lock_};
    if (!initialized_) return;
    initialized_ = false;
    for (auto* event_logger : event_loggers_) {
      event_logger->SetEventLoggerNotifier(nullptr);
      event_logger->SetPool(nullptr);
    }
  }
  if (options_.flush_all_events) {
    Flush({});
  } else {
    Clear();
  }
  options_.flush_queue->DrainAndStop();
  pool_ = nullptr;
}

auto RuntimeManager::Enable() -> void {
  std::unique_lock lock{lock_};
  if (!initialized_) return;
  enabled_ = true;
}

auto RuntimeManager::Disable() -> void {
  std::unique_lock lock{lock_};
  enabled_ = false;
}

auto RuntimeManager::Subscribe(
    gsl::not_null<event_logger::EventLogger*> subscriber) -> void {
  std::unique_lock lock{lock_};
  event_loggers_.insert(subscriber);
  subscriber->SetPool(pool_.get());
}

auto RuntimeManager::Unsubscribe(
    gsl::not_null<event_logger::EventLogger*> subscriber) -> void {
  std::unique_lock lock{lock_};
  subscriber->SetPool(nullptr);
  event_loggers_.erase(subscriber);
}

auto RuntimeManager::LogEvent(const trace::Event event) -> void {
  thread_local event_logger::EventLogger event_logger{
      {.flush_queue = options_.flush_queue,
       .preferred_capacity = options_.thread_event_buffer_capacity,
       .flush_buffer_when_full = options_.flush_all_events},
      this};
  if (!Enabled()) return;
  event_logger.LogEvent(event);
}

auto RuntimeManager::LogEvent(
    const trace::EventType type,
    const trace::TimestampNanoseconds steady_clock_timestamp,
    const uint64 payload_1, const uint32 payload_2) -> void {
  LogEvent({.steady_clock_timestamp = steady_clock_timestamp,
            .payload_1 = payload_1,
            .type = type,
            .payload_2 = payload_2});
}

auto RuntimeManager::LogEvent(const trace::EventType type,
                              const uint64 payload_1, const uint32 payload_2)
    -> void {
  const auto now = options_.steady_clock->Now();
  const auto now_nanoseconds =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          now.time_since_epoch())
          .count();
  LogEvent(type, now_nanoseconds, payload_1, payload_2);
}

auto RuntimeManager::LogFunctionEntry(const trace::FunctionId function_id)
    -> void {
  LogEvent(static_cast<trace::EventType>(trace::Event::Type::kFunctionEntry),
           function_id, 0);
}

auto RuntimeManager::LogFunctionExit(const trace::FunctionId function_id)
    -> void {
  LogEvent(static_cast<trace::EventType>(trace::Event::Type::kFunctionExit),
           function_id, 0);
}

auto RuntimeManager::Flush(std::function<void()> completion) -> void {
  {
    std::unique_lock lock{lock_};
    for (auto* event_logger : event_loggers_) {
      event_logger->Flush();
    }
  }
  options_.flush_queue->Flush(std::move(completion));
}

auto RuntimeManager::Clear() -> void {
  {
    std::unique_lock lock{lock_};
    for (auto* event_logger : event_loggers_) {
      event_logger->Clear();
    }
  }
  options_.flush_queue->Clear();
}

auto RuntimeManager::Initialized() const -> bool {
  std::shared_lock lock{lock_};
  return initialized_;
}

auto RuntimeManager::Enabled() const -> bool {
  std::shared_lock lock{lock_};
  return enabled_;
}

auto operator==(const RuntimeManager::DeletedFilesInfo& lhs,
                const RuntimeManager::DeletedFilesInfo& rhs) -> bool {
  return lhs.deleted_files == rhs.deleted_files &&
         lhs.deleted_bytes == rhs.deleted_bytes;
}

}  // namespace spoor::runtime::runtime_manager
