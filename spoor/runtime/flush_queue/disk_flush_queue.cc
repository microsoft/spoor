// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/flush_queue/disk_flush_queue.h"

#include <chrono>
#include <filesystem>
#include <functional>
#include <mutex>
#include <queue>
#include <shared_mutex>
#include <thread>

#include "absl/strings/str_format.h"
#include "gsl/gsl"
#include "spoor/runtime/trace/trace.h"

namespace spoor::runtime::flush_queue {

using std::literals::chrono_literals::operator""ns;

DiskFlushQueue::DiskFlushQueue(Options options)
    : options_{std::move(options)},
      flush_timestamp_{std::chrono::time_point<std::chrono::steady_clock>{0ns}},
      next_flush_info_id_{0},
      queue_size_{0},
      running_{false},
      draining_{false} {}

DiskFlushQueue::~DiskFlushQueue() { DrainAndStop(); }

auto DiskFlushQueue::Run() -> void {
  if (running_.exchange(true)) return;
  draining_ = false;
  flush_thread_ = std::thread{[&] {
    while (!draining_ || !Empty()) {
      auto flush_info_optional = [&]() -> std::optional<FlushInfo> {
        std::unique_lock lock{lock_};
        if (queue_.empty()) return {};
        auto flush_info = std::move(queue_.front());
        queue_.pop_front();
        return flush_info;
      }();
      if (!flush_info_optional.has_value()) {
        std::this_thread::yield();
        continue;
      };
      auto flush_info = std::move(flush_info_optional.value());
      const bool retain{options_.steady_clock->Now() <
                        flush_info.flush_timestamp +
                            options_.buffer_retention_duration};
      if (!options_.flush_all_events && !retain) {
        std::unique_lock lock{lock_};
        manual_flush_record_ids_.erase(flush_info.id);
        --queue_size_;
        continue;
      }
      const auto flush = [&] {
        if (options_.flush_all_events) return true;
        std::shared_lock lock{lock_};
        return flush_info.flush_timestamp <= flush_timestamp_;
      }();
      if (!flush) {
        std::unique_lock lock{lock_};
        queue_.emplace_back(std::move(flush_info));
        continue;
      }
      const auto result = options_.trace_writer->Write(
          TraceFilePath(flush_info), TraceFileHeader(flush_info),
          &flush_info.buffer, trace::Footer{});
      --flush_info.remaining_flush_attempts;
      if (result.IsErr() && 0 < flush_info.remaining_flush_attempts) {
        std::unique_lock lock{lock_};
        queue_.emplace_back(std::move(flush_info));
      } else {
        std::unique_lock lock{lock_};
        manual_flush_record_ids_.erase(flush_info.id);
        --queue_size_;
      }
      std::optional<std::function<void()>> flush_completion{};
      {
        std::unique_lock lock{lock_};
        if (manual_flush_record_ids_.empty()) {
          flush_completion_.swap(flush_completion);
        }
      }
      if (flush_completion.has_value() && flush_completion.value() != nullptr) {
        std::thread{[completion{std::move(flush_completion.value())}] {
          completion();
        }}.detach();
      }
    }
    draining_ = false;
    running_ = false;
  }};
}

auto DiskFlushQueue::DrainAndStop() -> void {
  if (!running_ || draining_.exchange(true)) return;
  if (flush_thread_.joinable()) flush_thread_.join();
}

auto DiskFlushQueue::Enqueue(Buffer&& buffer) -> void {
  const auto flush_timestamp = options_.steady_clock->Now();
  if (!running_ || draining_) return;
  const auto thread_id = static_cast<trace::ThreadId>(
      std::hash<std::thread::id>{}(std::this_thread::get_id()));
  std::unique_lock lock{lock_};
  const auto id = next_flush_info_id_;
  ++next_flush_info_id_;
  FlushInfo flush_info{
      .buffer = std::move(buffer),
      .flush_timestamp = flush_timestamp,
      .thread_id = thread_id,
      .id = id,
      .remaining_flush_attempts = options_.max_buffer_flush_attempts};
  queue_.emplace_back(std::move(flush_info));
  ++queue_size_;
}

auto DiskFlushQueue::Flush(std::function<void()> completion) -> void {
  const auto now = options_.steady_clock->Now();
  std::unique_lock lock{lock_};
  flush_timestamp_ = now;
  if (completion != nullptr) {
    flush_completion_ = completion;
    for (const auto& record : queue_) {
      if (record.flush_timestamp <= now) {
        manual_flush_record_ids_.emplace(record.id);
      }
    }
  }
}

auto DiskFlushQueue::Clear() -> void {
  std::deque<FlushInfo> empty{};
  std::unique_lock lock{lock_};
  std::swap(queue_, empty);
  manual_flush_record_ids_.clear();
  queue_size_ = 0;
}

auto DiskFlushQueue::Size() const -> DiskFlushQueue::SizeType {
  return queue_size_;
}

auto DiskFlushQueue::Empty() const -> bool { return queue_size_ == 0; }

auto DiskFlushQueue::TraceFilePath(const FlushInfo& flush_info) const
    -> std::filesystem::path {
  const auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                             flush_info.flush_timestamp.time_since_epoch())
                             .count();
  const auto file_name =
      absl::StrFormat("%016x-%016x-%016x.%s", options_.session_id,
                      flush_info.thread_id, timestamp, kTraceFileExtension);
  return options_.trace_file_path / file_name;
}

auto DiskFlushQueue::TraceFileHeader(const FlushInfo& flush_info) const
    -> trace::Header {
  const auto system_clock_now = options_.system_clock->Now();
  const auto steady_clock_now = options_.steady_clock->Now();
  const auto system_clock_timestamp =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          system_clock_now.time_since_epoch())
          .count();
  const auto steady_clock_timestamp =
      std::chrono::duration_cast<std::chrono::nanoseconds>(
          steady_clock_now.time_since_epoch())
          .count();
  const auto event_count =
      gsl::narrow_cast<trace::EventCount>(flush_info.buffer.Size());
  return trace::Header{.version = trace::kTraceFileVersion,
                       .session_id = options_.session_id,
                       .process_id = options_.process_id,
                       .thread_id = flush_info.thread_id,
                       .system_clock_timestamp = system_clock_timestamp,
                       .steady_clock_timestamp = steady_clock_timestamp,
                       .event_count = event_count};
}

}  // namespace spoor::runtime::flush_queue
