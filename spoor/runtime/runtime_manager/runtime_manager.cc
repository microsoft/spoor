#include "spoor/runtime/runtime_manager/runtime_manager.h"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <mutex>
#include <regex>
#include <shared_mutex>
#include <thread>
#include <utility>

#include "gsl/gsl"

namespace spoor::runtime::runtime_manager {

const std::regex kTraceFileNamePattern{
    R"([0-9a-f]{16}-[0-9a-f]{16}-[0-9a-f]{16}\.spoor)"};

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
  if (options_.flush_all_events) {
    Flush({});
  } else {
    Clear({});
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

auto RuntimeManager::LogEvent(const trace::Event::Type type,
                              const trace::FunctionId function_id) -> void {
  thread_local event_logger::EventLogger event_logger{
      {.steady_clock = options_.steady_clock,
       .event_logger_notifier = this,
       .flush_queue = options_.flush_queue,
       .preferred_capacity = options_.thread_event_buffer_capacity,
       .flush_buffer_when_full = options_.flush_all_events}};
  if (!Enabled()) return;
  event_logger.LogEvent(type, function_id);
}

auto RuntimeManager::Flush(const std::function<void()> completion) -> void {
  std::thread{[&] {
    std::unique_lock lock{lock_};
    for (auto* event_logger : event_loggers_) {
      event_logger->Flush();
    }
    options_.flush_queue->Flush([&] {
      if (completion != nullptr) completion();
    });
  }}.detach();  // TODO
}

auto RuntimeManager::Clear(const std::function<void()> completion) -> void {
  std::thread{[&] {
    std::unique_lock lock{lock_};
    for (auto* event_logger : event_loggers_) {
      event_logger->Clear();
    }
    options_.flush_queue->Clear();
    if (completion != nullptr) completion();
  }}.detach();  // TODO
}

auto RuntimeManager::Initialized() const -> bool {
  std::shared_lock lock{lock_};
  return initialized_;
}

auto RuntimeManager::Enabled() const -> bool {
  std::shared_lock lock{lock_};
  return enabled_;
}

auto RuntimeManager::FlushedTraceFiles(
    std::function<void(std::vector<std::filesystem::path>)> completion) const
    -> void {
  std::thread{[trace_file_path{options_.trace_file_path},
               completion{std::move(completion)}] {
    std::vector<std::filesystem::path> trace_file_paths{};
    auto directory_iterator =
        std::filesystem::directory_iterator(trace_file_path);
    std::copy_if(std::filesystem::begin(directory_iterator),
                 std::filesystem::end(directory_iterator),
                 std::back_inserter(trace_file_paths), [](const auto& file) {
                   if (!file.is_regular_file()) return false;
                   return std::regex_match(file.path().filename().string(),
                                           kTraceFileNamePattern);
                 });
    completion(std::move(trace_file_paths));
  }}.detach();
}

auto RuntimeManager::DeleteFlushedTraceFilesOlderThan(
    const std::chrono::time_point<std::chrono::system_clock> timestamp,
    std::function<void(DeletedFilesInfo)> completion) const -> void {
  std::thread{[trace_file_path{options_.trace_file_path}, timestamp,
               completion{std::move(completion)}] {
    DeletedFilesInfo deleted_files_info {
      .deleted_files = 0,
      .deleted_bytes = 0};
    for (const auto& file :
         std::filesystem::directory_iterator(trace_file_path)) {
      if (!file.is_regular_file() ||
          !std::regex_match(file.path().filename().string(),
                            kTraceFileNamePattern)) {
        continue;
      }
      {
        std::ifstream file_stream{file};
        if (!file_stream.is_open()) continue;
        std::array<char, sizeof(spoor::runtime::trace::Header)> header_buffer{};
        file_stream.read(header_buffer.data(), header_buffer.size());
        const auto header = spoor::runtime::trace::Deserialize(header_buffer);
        if (header.version != spoor::runtime::trace::kTraceFileVersion) {
          continue;
        }
        const auto header_system_clock_timestamp =
            std::chrono::time_point<std::chrono::system_clock>{
              std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::nanoseconds{header.system_clock_timestamp})};
        if (timestamp < header_system_clock_timestamp) continue;
      }
      const auto file_size = std::filesystem::file_size(file.path());
      const auto success = std::filesystem::remove(file.path());
      if (success) {
        ++deleted_files_info.deleted_files;
        deleted_files_info.deleted_bytes += file_size;
      }
    }
    if (completion != nullptr) completion(deleted_files_info);
  }}.detach();
}

}  // namespace spoor::runtime::runtime_manager
