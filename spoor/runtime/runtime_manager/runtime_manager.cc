#include "spoor/runtime/runtime_manager/runtime_manager.h"

#include <memory>
#include <mutex>
#include <shared_mutex>

#include "gsl/gsl"

namespace spoor::runtime::runtime_manager {

RuntimeManager::RuntimeManager(Options options)
    : options_{std::move(options)},
      pool_{},
      lock_{},
      event_loggers_{},
      initialized_{false},
      enabled_{false} {}

RuntimeManager::~RuntimeManager() { Deinitialize(); }

auto RuntimeManager::Initialize() -> void {
  std::unique_lock lock{lock_};
  if (initialized_) return;
  initialized_ = true;
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
}

auto RuntimeManager::Deinitialize() -> void {
  Disable();
  std::unique_lock lock{lock_};
  if (!initialized_) return;
  initialized_ = false;
  enabled_ = false;
  for (auto* event_logger : event_loggers_) {
    event_logger->SetPool(nullptr);
  }
  if (options_.flush_immediately) {
    options_.flush_queue->Flush();
  } else {
    options_.flush_queue->Clear();
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
}

auto RuntimeManager::Unsubscribe(
    gsl::not_null<event_logger::EventLogger*> subscriber) -> void {
  std::unique_lock lock{lock_};
  event_loggers_.erase(subscriber);
}

auto RuntimeManager::Flush() -> void {
  {
    std::unique_lock lock{lock_};
    for (auto* event_logger : event_loggers_) {
      event_logger->Flush();
    }
  }
  options_.flush_queue->Flush();
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

}  // namespace spoor::runtime::runtime_manager
