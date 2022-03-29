// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/config/file_source.h"

#include <algorithm>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/strings/ascii.h"
#include "absl/strings/str_format.h"
#include "spoor/runtime/config/source.h"
#include "spoor/runtime/trace/trace.h"
#include "tomlplusplus/toml.h"
#include "util/compression/compressor.h"
#include "util/numeric.h"

namespace spoor::runtime::config {

template <class T>
auto ReadConfig(const toml::table& table, const std::string_view key,
                gsl::not_null<std::vector<Source::ReadError>*> errors)
    -> std::optional<T> {
  const auto* table_value = table.get(key);
  if (table_value == nullptr) return {};
  auto value = table_value->value<T>();
  if (!value.has_value()) {
    errors->push_back({
        .type = Source::ReadError::Type::kUnknownValue,
        .message = absl::StrFormat("Cannot parse value for key \"%s\".", key),
    });
  }
  return value;
}

FileSource::FileSource(Options options)
    : options_{std::move(options)}, read_{false} {}

auto FileSource::Read() -> std::vector<ReadError> {
  auto finally_set_read = gsl::finally([this] { read_ = true; });

  auto& file = *options_.file_reader;
  file.Open(options_.file_path);
  if (!file.IsOpen()) {
    return {{
        .type = ReadError::Type::kFailedToOpenFile,
        .message = absl::StrFormat(
            "Failed to open the file \"%s\" for reading.", options_.file_path),
    }};
  }
  auto finally_close_file = gsl::finally([&file] { file.Close(); });

  auto toml_result = toml::parse(file.Istream());
  if (!toml_result) {
    return {{
        .type = ReadError::Type::kMalformedFile,
        .message{toml_result.error().description()},
    }};
  }
  const auto table = std::move(toml_result).table();

  std::vector<ReadError> errors{};

  for (const auto& [key, node] : table) {
    if (std::find(std::cbegin(kFileConfigKeys), std::cend(kFileConfigKeys),
                  key) == std::cend(kFileConfigKeys)) {
      errors.push_back({
          .type = ReadError::Type::kUnknownKey,
          .message = absl::StrFormat("Unknown key \"%s\".", key),
      });
    }
  }

  trace_file_path_ = ReadConfig<decltype(trace_file_path_)::value_type>(
      table, kTraceFilePathFileKey, &errors);
  compression_strategy_ = [&]() -> decltype(compression_strategy_) {
    const auto compression_strategy_user_value =
        ReadConfig<std::string>(table, kCompressionStrategyFileKey, &errors);
    if (!compression_strategy_user_value.has_value()) return {};
    auto normalized_value = compression_strategy_user_value.value();
    absl::StripAsciiWhitespace(&normalized_value);
    absl::AsciiStrToLower(&normalized_value);
    const auto compression_strategy =
        util::compression::kStrategyMap.FirstValueForKey(normalized_value);
    if (!compression_strategy.has_value()) {
      errors.push_back({
          .type = ReadError::Type::kUnknownValue,
          .message = absl::StrFormat("Unknown compression strategy \"%s\".",
                                     compression_strategy_user_value.value()),
      });
    }
    return compression_strategy;
  }();
  session_id_ = ReadConfig<decltype(session_id_)::value_type>(
      table, kSessionIdFileKey, &errors);
  thread_event_buffer_capacity_ =
      ReadConfig<decltype(thread_event_buffer_capacity_)::value_type>(
          table, kThreadEventBufferCapacityFileKey, &errors);
  max_reserved_event_buffer_slice_capacity_ = ReadConfig<
      decltype(max_reserved_event_buffer_slice_capacity_)::value_type>(
      table, kMaxReservedEventBufferSliceCapacityFileKey, &errors);
  max_dynamic_event_buffer_slice_capacity_ = ReadConfig<
      decltype(max_dynamic_event_buffer_slice_capacity_)::value_type>(
      table, kMaxDynamicEventBufferSliceCapacityFileKey, &errors);
  reserved_event_pool_capacity_ =
      ReadConfig<decltype(reserved_event_pool_capacity_)::value_type>(
          table, kReservedEventPoolCapacityFileKey, &errors);
  dynamic_event_pool_capacity_ =
      ReadConfig<decltype(dynamic_event_pool_capacity_)::value_type>(
          table, kDynamicEventPoolCapacityFileKey, &errors);
  dynamic_event_slice_borrow_cas_attempts_ = ReadConfig<
      decltype(dynamic_event_slice_borrow_cas_attempts_)::value_type>(
      table, kDynamicEventSliceBorrowCasAttemptsFileKey, &errors);
  event_buffer_retention_duration_nanoseconds_ = ReadConfig<
      decltype(event_buffer_retention_duration_nanoseconds_)::value_type>(
      table, kEventBufferRetentionDurationNanosecondsFileKey, &errors);
  max_flush_buffer_to_file_attempts_ =
      ReadConfig<decltype(max_flush_buffer_to_file_attempts_)::value_type>(
          table, kMaxFlushBufferToFileAttemptsFileKey, &errors);
  flush_all_events_ = ReadConfig<decltype(flush_all_events_)::value_type>(
      table, kFlushAllEventsFileKey, &errors);

  return errors;
}

auto FileSource::IsRead() const -> bool { return read_; }

auto FileSource::TraceFilePath() const -> std::optional<std::string> {
  return trace_file_path_;
}

auto FileSource::CompressionStrategy() const
    -> std::optional<util::compression::Strategy> {
  return compression_strategy_;
}

auto FileSource::SessionId() const -> std::optional<trace::SessionId> {
  return session_id_;
}

auto FileSource::ThreadEventBufferCapacity() const -> std::optional<SizeType> {
  return thread_event_buffer_capacity_;
}

auto FileSource::MaxReservedEventBufferSliceCapacity() const
    -> std::optional<SizeType> {
  return max_reserved_event_buffer_slice_capacity_;
}

auto FileSource::MaxDynamicEventBufferSliceCapacity() const
    -> std::optional<SizeType> {
  return max_dynamic_event_buffer_slice_capacity_;
}

auto FileSource::ReservedEventPoolCapacity() const -> std::optional<SizeType> {
  return reserved_event_pool_capacity_;
}

auto FileSource::DynamicEventPoolCapacity() const -> std::optional<SizeType> {
  return dynamic_event_pool_capacity_;
}

auto FileSource::DynamicEventSliceBorrowCasAttempts() const
    -> std::optional<SizeType> {
  return dynamic_event_slice_borrow_cas_attempts_;
}

auto FileSource::EventBufferRetentionDurationNanoseconds() const
    -> std::optional<trace::DurationNanoseconds> {
  return event_buffer_retention_duration_nanoseconds_;
}

auto FileSource::MaxFlushBufferToFileAttempts() const -> std::optional<int32> {
  return max_flush_buffer_to_file_attempts_;
}

auto FileSource::FlushAllEvents() const -> std::optional<bool> {
  return flush_all_events_;
}

}  // namespace spoor::runtime::config
