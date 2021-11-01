// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/config/config.h"

#include "util/compression/compressor.h"
#include "util/env/env.h"
#include "util/file_system/util.h"
#include "util/numeric.h"

namespace spoor::runtime::config {

using util::env::GetEnv;
using util::file_system::ExpandTilde;
using util::result::None;

auto Config::FromEnv(const util::env::StdGetEnv& get_env) -> Config {
  constexpr auto normalize{true};
  constexpr auto empty_string_is_nullopt{true};
  // Temporarily returning an `Err` is okay because it immediately gets
  // converted to the default value -- the desired behavior if the key doesn't
  // exist.
  const auto trace_file_path =
      GetEnv(kTraceFilePathKey, empty_string_is_nullopt, get_env)
          .value_or(util::result::Result<std::string, None>::Err({}))
          .OkOr(std::string{kTraceFilePathDefaultValue});
  const auto compression_strategy =
      GetEnv(kCompressionStrategyKey, kCompressionStrategies, normalize,
             get_env)
          .value_or(
              util::result::Result<util::compression::Strategy, None>::Err({}))
          .OkOr(kCompressionStrategyDefaultValue);
  const auto session_id =
      GetEnv<trace::SessionId>(kSessionIdKey, get_env)
          .value_or(util::result::Result<trace::SessionId, None>::Err({}))
          .OkOr(kSessionIdDefaultValue());
  const auto thread_event_buffer_capacity =
      GetEnv<SizeType>(kThreadEventBufferCapacityKey, get_env)
          .value_or(util::result::Result<SizeType, None>::Err({}))
          .OkOr(kThreadEventBufferCapacityDefaultValue);
  const auto max_reserved_event_buffer_slice_capacity =
      GetEnv<SizeType>(kMaxReservedEventBufferSliceCapacityKey, get_env)
          .value_or(util::result::Result<SizeType, None>::Err({}))
          .OkOr(kMaxReservedEventBufferSliceCapacityDefaultValue);
  const auto max_dynamic_event_buffer_slice_capacity =
      GetEnv<SizeType>(kMaxDynamicEventBufferSliceCapacityKey, get_env)
          .value_or(util::result::Result<SizeType, None>::Err({}))
          .OkOr(kMaxDynamicEventBufferSliceCapacityDefaultValue);
  const auto reserved_event_pool_capacity =
      GetEnv<SizeType>(kReservedEventPoolCapacityKey, get_env)
          .value_or(util::result::Result<SizeType, None>::Err({}))
          .OkOr(kReservedEventPoolCapacityDefaultValue);
  const auto dynamic_event_pool_capacity =
      GetEnv<SizeType>(kDynamicEventPoolCapacityKey, get_env)
          .value_or(util::result::Result<SizeType, None>::Err({}))
          .OkOr(kDynamicEventPoolCapacityDefaultValue);
  const auto dynamic_event_slice_borrow_cas_attempts =
      GetEnv<SizeType>(kDynamicEventSliceBorrowCasAttemptsKey, get_env)
          .value_or(util::result::Result<SizeType, None>::Err({}))
          .OkOr(kDynamicEventSliceBorrowCasAttemptsDefaultValue);
  const auto event_buffer_retention_duration_nanoseconds =
      GetEnv<trace::DurationNanoseconds>(
          kEventBufferRetentionDurationNanosecondsKey, get_env)
          .value_or(
              util::result::Result<trace::DurationNanoseconds, None>::Err({}))
          .OkOr(kEventBufferRetentionNanosecondsDefaultValue);
  const auto max_flush_buffer_to_file_attempts =
      GetEnv<int32>(kMaxFlushBufferToFileAttemptsKey, get_env)
          .value_or(util::result::Result<int32, None>::Err({}))
          .OkOr(kMaxFlushBufferToFileAttemptsDefaultValue);
  const auto flush_all_events =
      GetEnv<bool>(kFlushAllEventsKey, get_env)
          .value_or(util::result::Result<bool, None>::Err({}))
          .OkOr(kFlushAllEventsDefaultValue);
  return {
      .trace_file_path = ExpandTilde(trace_file_path, get_env),
      .compression_strategy = compression_strategy,
      .session_id = session_id,
      .thread_event_buffer_capacity = thread_event_buffer_capacity,
      .max_reserved_event_buffer_slice_capacity =
          max_reserved_event_buffer_slice_capacity,
      .max_dynamic_event_buffer_slice_capacity =
          max_dynamic_event_buffer_slice_capacity,
      .reserved_event_pool_capacity = reserved_event_pool_capacity,
      .dynamic_event_pool_capacity = dynamic_event_pool_capacity,
      .dynamic_event_slice_borrow_cas_attempts =
          dynamic_event_slice_borrow_cas_attempts,
      .event_buffer_retention_duration_nanoseconds =
          event_buffer_retention_duration_nanoseconds,
      .max_flush_buffer_to_file_attempts = max_flush_buffer_to_file_attempts,
      .flush_all_events = flush_all_events,
  };
}

auto operator==(const Config& lhs, const Config& rhs) -> bool {
  return lhs.trace_file_path == rhs.trace_file_path &&
         lhs.compression_strategy == rhs.compression_strategy &&
         lhs.session_id == rhs.session_id &&
         lhs.thread_event_buffer_capacity == rhs.thread_event_buffer_capacity &&
         lhs.max_reserved_event_buffer_slice_capacity ==
             rhs.max_reserved_event_buffer_slice_capacity &&
         lhs.max_dynamic_event_buffer_slice_capacity ==
             rhs.max_dynamic_event_buffer_slice_capacity &&
         lhs.reserved_event_pool_capacity == rhs.reserved_event_pool_capacity &&
         lhs.dynamic_event_pool_capacity == rhs.dynamic_event_pool_capacity &&
         lhs.dynamic_event_slice_borrow_cas_attempts ==
             rhs.dynamic_event_slice_borrow_cas_attempts &&
         lhs.event_buffer_retention_duration_nanoseconds ==
             rhs.event_buffer_retention_duration_nanoseconds &&
         lhs.flush_all_events == rhs.flush_all_events;
}

}  // namespace spoor::runtime::config
