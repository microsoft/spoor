// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/config/env_source.h"

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "gsl/gsl"
#include "spoor/runtime/config/source.h"
#include "spoor/runtime/trace/trace.h"
#include "util/env/env.h"
#include "util/numeric.h"
#include "util/result.h"

namespace spoor::runtime::config {

using util::env::GetEnv;

EnvSource::EnvSource(Options options)
    : options_{std::move(options)}, read_{false} {}

template <class T>
auto ReadConfig(
    const std::string_view key,
    gsl::not_null<std::vector<Source::ReadError>*> errors,
    std::function<std::optional<util::result::Result<T, util::result::None>>(
        std::string_view)>
        get_env) -> std::optional<T> {
  const auto value = get_env(key);
  if (!value.has_value()) return {};
  if (value.value().IsErr()) {
    errors->push_back({
        .type = Source::ReadError::Type::kUnknownValue,
        .message = absl::StrFormat("Cannot parse value for key \"%s\".", key),
    });
    return {};
  }
  return value.value().Ok();
}

auto EnvSource::Read() -> std::vector<ReadError> {
  constexpr auto empty_string_is_nullopt{true};
  constexpr auto normalize{true};
  auto finally = gsl::finally([this] { read_ = true; });

  std::vector<ReadError> errors{};

  trace_file_path_ = ReadConfig<decltype(trace_file_path_)::value_type>(
      kTraceFilePathEnvKey, &errors, [&](const auto key) {
        return GetEnv(key, empty_string_is_nullopt, options_.get_env);
      });
  compression_strategy_ =
      ReadConfig<decltype(compression_strategy_)::value_type>(
          kCompressionStrategyEnvKey, &errors, [&](const auto key) {
            return GetEnv(key, util::compression::kStrategyMap, normalize,
                          options_.get_env);
          });
  session_id_ = ReadConfig<decltype(session_id_)::value_type>(
      kSessionIdEnvKey, &errors, [&](const auto key) {
        return GetEnv<decltype(session_id_)::value_type>(key, options_.get_env);
      });
  thread_event_buffer_capacity_ =
      ReadConfig<decltype(thread_event_buffer_capacity_)::value_type>(
          kThreadEventBufferCapacityEnvKey, &errors, [&](const auto key) {
            return GetEnv<decltype(thread_event_buffer_capacity_)::value_type>(
                key, options_.get_env);
          });
  max_reserved_event_buffer_slice_capacity_ = ReadConfig<
      decltype(max_reserved_event_buffer_slice_capacity_)::value_type>(
      kMaxReservedEventBufferSliceCapacityEnvKey, &errors, [&](const auto key) {
        return GetEnv<
            decltype(max_reserved_event_buffer_slice_capacity_)::value_type>(
            key, options_.get_env);
      });
  max_dynamic_event_buffer_slice_capacity_ = ReadConfig<
      decltype(max_dynamic_event_buffer_slice_capacity_)::value_type>(
      kMaxDynamicEventBufferSliceCapacityEnvKey, &errors, [&](const auto key) {
        return GetEnv<
            decltype(max_dynamic_event_buffer_slice_capacity_)::value_type>(
            key, options_.get_env);
      });
  reserved_event_pool_capacity_ =
      ReadConfig<decltype(reserved_event_pool_capacity_)::value_type>(
          kReservedEventPoolCapacityEnvKey, &errors, [&](const auto key) {
            return GetEnv<decltype(reserved_event_pool_capacity_)::value_type>(
                key, options_.get_env);
          });
  dynamic_event_pool_capacity_ =
      ReadConfig<decltype(dynamic_event_pool_capacity_)::value_type>(
          kDynamicEventPoolCapacityEnvKey, &errors, [&](const auto key) {
            return GetEnv<decltype(dynamic_event_pool_capacity_)::value_type>(
                key, options_.get_env);
          });
  dynamic_event_slice_borrow_cas_attempts_ = ReadConfig<
      decltype(dynamic_event_slice_borrow_cas_attempts_)::value_type>(
      kDynamicEventSliceBorrowCasAttemptsEnvKey, &errors, [&](const auto key) {
        return GetEnv<
            decltype(dynamic_event_slice_borrow_cas_attempts_)::value_type>(
            key, options_.get_env);
      });
  event_buffer_retention_duration_nanoseconds_ = ReadConfig<
      decltype(event_buffer_retention_duration_nanoseconds_)::value_type>(
      kEventBufferRetentionDurationNanosecondsEnvKey, &errors,
      [&](const auto key) {
        return GetEnv<
            decltype(event_buffer_retention_duration_nanoseconds_)::value_type>(
            key, options_.get_env);
      });
  max_flush_buffer_to_file_attempts_ = ReadConfig<
      decltype(max_flush_buffer_to_file_attempts_)::value_type>(
      kMaxFlushBufferToFileAttemptsEnvKey, &errors, [&](const auto key) {
        return GetEnv<decltype(max_flush_buffer_to_file_attempts_)::value_type>(
            key, options_.get_env);
      });
  flush_all_events_ = ReadConfig<decltype(flush_all_events_)::value_type>(
      kFlushAllEventsEnvKey, &errors, [&](const auto key) {
        return GetEnv<decltype(flush_all_events_)::value_type>(
            key, options_.get_env);
      });

  return errors;
}

auto EnvSource::IsRead() const -> bool { return read_; }

auto EnvSource::TraceFilePath() const -> std::optional<std::string> {
  return trace_file_path_;
}

auto EnvSource::CompressionStrategy() const
    -> std::optional<util::compression::Strategy> {
  return compression_strategy_;
}

auto EnvSource::SessionId() const -> std::optional<trace::SessionId> {
  return session_id_;
}

auto EnvSource::ThreadEventBufferCapacity() const -> std::optional<SizeType> {
  return thread_event_buffer_capacity_;
}

auto EnvSource::MaxReservedEventBufferSliceCapacity() const
    -> std::optional<SizeType> {
  return max_reserved_event_buffer_slice_capacity_;
}

auto EnvSource::MaxDynamicEventBufferSliceCapacity() const
    -> std::optional<SizeType> {
  return max_dynamic_event_buffer_slice_capacity_;
}

auto EnvSource::ReservedEventPoolCapacity() const -> std::optional<SizeType> {
  return reserved_event_pool_capacity_;
}

auto EnvSource::DynamicEventPoolCapacity() const -> std::optional<SizeType> {
  return dynamic_event_pool_capacity_;
}

auto EnvSource::DynamicEventSliceBorrowCasAttempts() const
    -> std::optional<SizeType> {
  return dynamic_event_slice_borrow_cas_attempts_;
}

auto EnvSource::EventBufferRetentionDurationNanoseconds() const
    -> std::optional<trace::DurationNanoseconds> {
  return event_buffer_retention_duration_nanoseconds_;
}

auto EnvSource::MaxFlushBufferToFileAttempts() const -> std::optional<int32> {
  return max_flush_buffer_to_file_attempts_;
}

auto EnvSource::FlushAllEvents() const -> std::optional<bool> {
  return flush_all_events_;
}

}  // namespace spoor::runtime::config
