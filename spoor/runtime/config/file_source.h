// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <array>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "spoor/runtime/config/source.h"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor.h"
#include "util/file_system/file_reader.h"
#include "util/numeric.h"

namespace spoor::runtime::config {

constexpr std::string_view kTraceFilePathFileKey{"trace_file_path"};
constexpr std::string_view kCompressionStrategyFileKey{"compression_strategy"};
constexpr std::string_view kSessionIdFileKey{"session_id"};
constexpr std::string_view kThreadEventBufferCapacityFileKey{
    "thread_event_buffer_capacity"};
constexpr std::string_view kMaxReservedEventBufferSliceCapacityFileKey{
    "max_reserved_event_buffer_slice_capacity"};
constexpr std::string_view kMaxDynamicEventBufferSliceCapacityFileKey{
    "max_dynamic_event_buffer_slice_capacity"};
constexpr std::string_view kReservedEventPoolCapacityFileKey{
    "reserved_event_pool_capacity"};
constexpr std::string_view kDynamicEventPoolCapacityFileKey{
    "dynamic_event_pool_capacity"};
constexpr std::string_view kDynamicEventSliceBorrowCasAttemptsFileKey{
    "dynamic_event_slice_borrow_cas_attempts"};
constexpr std::string_view kEventBufferRetentionDurationNanosecondsFileKey{
    "event_buffer_retention_duration_nanoseconds"};
constexpr std::string_view kMaxFlushBufferToFileAttemptsFileKey{
    "max_flush_buffer_to_file_attempts"};
constexpr std::string_view kFlushAllEventsFileKey{"flush_all_events"};
constexpr std::array<std::string_view, 12> kFileConfigKeys{{
    kTraceFilePathFileKey,
    kCompressionStrategyFileKey,
    kSessionIdFileKey,
    kThreadEventBufferCapacityFileKey,
    kMaxReservedEventBufferSliceCapacityFileKey,
    kMaxDynamicEventBufferSliceCapacityFileKey,
    kReservedEventPoolCapacityFileKey,
    kDynamicEventPoolCapacityFileKey,
    kDynamicEventSliceBorrowCasAttemptsFileKey,
    kEventBufferRetentionDurationNanosecondsFileKey,
    kMaxFlushBufferToFileAttemptsFileKey,
    kFlushAllEventsFileKey,
}};

class FileSource final : public Source {
 public:
  struct alignas(32) Options {
    std::unique_ptr<util::file_system::FileReader> file_reader;
    std::filesystem::path file_path;
  };

  FileSource() = delete;
  explicit FileSource(Options options);
  FileSource(const FileSource&) = delete;
  FileSource(FileSource&&) noexcept = default;
  auto operator=(const FileSource&) -> FileSource& = delete;
  auto operator=(FileSource&&) noexcept -> FileSource& = default;
  ~FileSource() override = default;

  [[nodiscard]] auto Read() -> std::vector<ReadError> override;
  [[nodiscard]] auto IsRead() const -> bool override;

  [[nodiscard]] auto TraceFilePath() const
      -> std::optional<std::string> override;
  [[nodiscard]] auto CompressionStrategy() const
      -> std::optional<util::compression::Strategy> override;
  [[nodiscard]] auto SessionId() const
      -> std::optional<trace::SessionId> override;
  [[nodiscard]] auto ThreadEventBufferCapacity() const
      -> std::optional<SizeType> override;
  [[nodiscard]] auto MaxReservedEventBufferSliceCapacity() const
      -> std::optional<SizeType> override;
  [[nodiscard]] auto MaxDynamicEventBufferSliceCapacity() const
      -> std::optional<SizeType> override;
  [[nodiscard]] auto ReservedEventPoolCapacity() const
      -> std::optional<SizeType> override;
  [[nodiscard]] auto DynamicEventPoolCapacity() const
      -> std::optional<SizeType> override;
  [[nodiscard]] auto DynamicEventSliceBorrowCasAttempts() const
      -> std::optional<SizeType> override;
  [[nodiscard]] auto EventBufferRetentionDurationNanoseconds() const
      -> std::optional<trace::DurationNanoseconds> override;
  [[nodiscard]] auto MaxFlushBufferToFileAttempts() const
      -> std::optional<int32> override;
  [[nodiscard]] auto FlushAllEvents() const -> std::optional<bool> override;

 private:
  Options options_;
  bool read_;

  std::optional<std::string> trace_file_path_;
  std::optional<util::compression::Strategy> compression_strategy_;
  std::optional<trace::SessionId> session_id_;
  std::optional<SizeType> thread_event_buffer_capacity_;
  std::optional<SizeType> max_reserved_event_buffer_slice_capacity_;
  std::optional<SizeType> max_dynamic_event_buffer_slice_capacity_;
  std::optional<SizeType> reserved_event_pool_capacity_;
  std::optional<SizeType> dynamic_event_pool_capacity_;
  std::optional<SizeType> dynamic_event_slice_borrow_cas_attempts_;
  std::optional<trace::DurationNanoseconds>
      event_buffer_retention_duration_nanoseconds_;
  std::optional<int32> max_flush_buffer_to_file_attempts_;
  std::optional<bool> flush_all_events_;
};

}  // namespace spoor::runtime::config
