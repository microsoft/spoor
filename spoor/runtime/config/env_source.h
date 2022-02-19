// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "spoor/runtime/config/source.h"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor.h"
#include "util/env/env.h"
#include "util/numeric.h"

namespace spoor::runtime::config {

constexpr std::string_view kTraceFilePathEnvKey{
    "SPOOR_RUNTIME_TRACE_FILE_PATH"};
constexpr std::string_view kCompressionStrategyEnvKey{
    "SPOOR_RUNTIME_COMPRESSION_STRATEGY"};
constexpr std::string_view kSessionIdEnvKey{"SPOOR_RUNTIME_SESSION_ID"};
constexpr std::string_view kThreadEventBufferCapacityEnvKey{
    "SPOOR_RUNTIME_THEAD_EVENT_BUFFER_CAPACITY"};
constexpr std::string_view kMaxReservedEventBufferSliceCapacityEnvKey{
    "SPOOR_RUNTIME_MAX_RESERVED_EVENT_BUFFER_SLICE_CAPACITY"};
constexpr std::string_view kMaxDynamicEventBufferSliceCapacityEnvKey{
    "SPOOR_RUNTIME_MAX_DYNAMIC_EVENT_BUFFER_SLICE_CAPACITY"};
constexpr std::string_view kReservedEventPoolCapacityEnvKey{
    "SPOOR_RUNTIME_RESERVED_EVENT_POOL_CAPACITY"};
constexpr std::string_view kDynamicEventPoolCapacityEnvKey{
    "SPOOR_RUNTIME_DYNAMIC_EVENT_POOL_CAPACITY"};
constexpr std::string_view kDynamicEventSliceBorrowCasAttemptsEnvKey{
    "SPOOR_RUNTIME_DYNAMIC_EVENT_SLICE_BORROW_CAS_ATTEMPTS"};
constexpr std::string_view kEventBufferRetentionDurationNanosecondsEnvKey{
    "SPOOR_RUNTIME_EVENT_BUFFER_RETENTION_DURATION_NANOSECONDS"};
constexpr std::string_view kMaxFlushBufferToFileAttemptsEnvKey{
    "SPOOR_RUNTIME_MAX_FLUSH_BUFFER_TO_FILE_ATTEMPTS"};
constexpr std::string_view kFlushAllEventsEnvKey{
    "SPOOR_RUNTIME_FLUSH_ALL_EVENTS"};
constexpr std::array<std::string_view, 12> kEnvConfigKeys{{
    kTraceFilePathEnvKey,
    kCompressionStrategyEnvKey,
    kSessionIdEnvKey,
    kThreadEventBufferCapacityEnvKey,
    kMaxReservedEventBufferSliceCapacityEnvKey,
    kMaxDynamicEventBufferSliceCapacityEnvKey,
    kReservedEventPoolCapacityEnvKey,
    kDynamicEventPoolCapacityEnvKey,
    kDynamicEventSliceBorrowCasAttemptsEnvKey,
    kEventBufferRetentionDurationNanosecondsEnvKey,
    kMaxFlushBufferToFileAttemptsEnvKey,
    kFlushAllEventsEnvKey,
}};

class EnvSource final : public Source {
 public:
  struct Options {
    util::env::StdGetEnv get_env;
  };

  EnvSource() = delete;
  explicit EnvSource(Options options);
  EnvSource(const EnvSource&) = delete;
  EnvSource(EnvSource&&) noexcept = default;
  auto operator=(const EnvSource&) -> EnvSource& = delete;
  auto operator=(EnvSource&&) noexcept -> EnvSource& = default;
  ~EnvSource() override = default;

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
