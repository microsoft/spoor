// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/config/config.h"

#include <functional>
#include <limits>
#include <memory>
#include <random>
#include <string_view>
#include <vector>

#include "absl/strings/match.h"
#include "spoor/common/config/util.h"
#include "spoor/runtime/config/source.h"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor.h"
#include "util/env/env.h"
#include "util/file_system/util.h"
#include "util/numeric.h"

namespace spoor::runtime::config {

using spoor::common::util::ValueFromSourceOrDefault;
using util::file_system::ExpandPath;
using util::file_system::PathExpansionOptions;

constexpr std::string_view kTraceFilePathDefaultValue{"."};
constexpr auto kCompressionStrategyDefaultValue{
    util::compression::Strategy::kSnappy};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
constexpr auto kSessionIdDefaultValue = [] {
  std::random_device seed{};
  std::default_random_engine engine{seed()};
  std::uniform_int_distribution<trace::SessionId> distribution{};
  return distribution(engine);
};
constexpr Config::SizeType kThreadEventBufferCapacityDefaultValue{10'000};
constexpr Config::SizeType kMaxReservedEventBufferSliceCapacityDefaultValue{
    1'000};
constexpr Config::SizeType kMaxDynamicEventBufferSliceCapacityDefaultValue{
    1'000};
constexpr Config::SizeType kReservedEventPoolCapacityDefaultValue{0};
constexpr auto kDynamicEventPoolCapacityDefaultValue{
    std::numeric_limits<Config::SizeType>::max()};
constexpr Config::SizeType kDynamicEventSliceBorrowCasAttemptsDefaultValue{1};
constexpr trace::DurationNanoseconds
    kEventBufferRetentionNanosecondsDefaultValue{0};
constexpr int32 kMaxFlushBufferToFileAttemptsDefaultValue{2};
constexpr auto kFlushAllEventsDefaultValue{true};

auto Config::Default() -> Config {
  static const auto session_id = kSessionIdDefaultValue();
  return {
      .trace_file_path = kTraceFilePathDefaultValue,
      .compression_strategy = kCompressionStrategyDefaultValue,
      .session_id = session_id,
      .thread_event_buffer_capacity = kThreadEventBufferCapacityDefaultValue,
      .max_reserved_event_buffer_slice_capacity =
          kMaxReservedEventBufferSliceCapacityDefaultValue,
      .max_dynamic_event_buffer_slice_capacity =
          kMaxDynamicEventBufferSliceCapacityDefaultValue,
      .reserved_event_pool_capacity = kReservedEventPoolCapacityDefaultValue,
      .dynamic_event_pool_capacity = kDynamicEventPoolCapacityDefaultValue,
      .dynamic_event_slice_borrow_cas_attempts =
          kDynamicEventSliceBorrowCasAttemptsDefaultValue,
      .event_buffer_retention_duration_nanoseconds =
          kEventBufferRetentionNanosecondsDefaultValue,
      .max_flush_buffer_to_file_attempts =
          kMaxFlushBufferToFileAttemptsDefaultValue,
      .flush_all_events = kFlushAllEventsDefaultValue,
  };
}

auto Config::FromSourcesOrDefault(
    std::vector<std::unique_ptr<Source>>&& sources,
    const Config& default_config,
    const PathExpansionOptions& path_expansion_options) -> Config {
  auto trace_file_path = [&] {
    auto path = ValueFromSourceOrDefault(sources, &Source::TraceFilePath,
                                         default_config.trace_file_path);
    path = ExpandPath(path, path_expansion_options);
    // std::filesystem expects directories to end with a trailing slash.
    const std::string separator{std::filesystem::path::preferred_separator};
    if (path != "." && !absl::EndsWith(path.string(), separator)) {
      path += separator;
    }
    return path;
  }();
  return {
      .trace_file_path = std::move(trace_file_path),
      .compression_strategy =
          ValueFromSourceOrDefault(sources, &Source::CompressionStrategy,
                                   default_config.compression_strategy),
      .session_id = ValueFromSourceOrDefault(sources, &Source::SessionId,
                                             default_config.session_id),
      .thread_event_buffer_capacity =
          ValueFromSourceOrDefault(sources, &Source::ThreadEventBufferCapacity,
                                   default_config.thread_event_buffer_capacity),
      .max_reserved_event_buffer_slice_capacity = ValueFromSourceOrDefault(
          sources, &Source::MaxReservedEventBufferSliceCapacity,
          default_config.max_reserved_event_buffer_slice_capacity),
      .max_dynamic_event_buffer_slice_capacity = ValueFromSourceOrDefault(
          sources, &Source::MaxDynamicEventBufferSliceCapacity,
          default_config.max_dynamic_event_buffer_slice_capacity),
      .reserved_event_pool_capacity =
          ValueFromSourceOrDefault(sources, &Source::ReservedEventPoolCapacity,
                                   default_config.reserved_event_pool_capacity),
      .dynamic_event_pool_capacity =
          ValueFromSourceOrDefault(sources, &Source::DynamicEventPoolCapacity,
                                   default_config.dynamic_event_pool_capacity),
      .dynamic_event_slice_borrow_cas_attempts = ValueFromSourceOrDefault(
          sources, &Source::DynamicEventSliceBorrowCasAttempts,
          default_config.dynamic_event_slice_borrow_cas_attempts),
      .event_buffer_retention_duration_nanoseconds = ValueFromSourceOrDefault(
          sources, &Source::EventBufferRetentionDurationNanoseconds,
          default_config.event_buffer_retention_duration_nanoseconds),
      .max_flush_buffer_to_file_attempts = ValueFromSourceOrDefault(
          sources, &Source::MaxFlushBufferToFileAttempts,
          default_config.max_flush_buffer_to_file_attempts),
      .flush_all_events = ValueFromSourceOrDefault(
          sources, &Source::FlushAllEvents, default_config.flush_all_events),
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
