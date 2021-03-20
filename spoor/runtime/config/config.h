// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <cstdlib>
#include <filesystem>
#include <limits>
#include <random>
#include <string>
#include <string_view>
#include <unordered_map>

#include "spoor/runtime/buffer/circular_buffer.h"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor.h"
#include "util/env/env.h"

namespace spoor::runtime::config {

using CompressionStrategy = util::compression::Strategy;

constexpr std::string_view kTraceFilePathKey{"SPOOR_RUNTIME_TRACE_FILE_PATH"};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::string kTraceFilePathDefaultValue{"."};
constexpr std::string_view kCompressionStrategyKey{
    "SPOOR_RUNTIME_COMPRESSION_STRATEGY"};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::unordered_map<std::string_view, CompressionStrategy>
    kCompressionStrategyMap{{"none", CompressionStrategy::kNone},
                            {"snappy", CompressionStrategy::kSnappy}};
constexpr auto kCompressionStrategyDefaultValue{CompressionStrategy::kSnappy};
constexpr std::string_view kSessionIdKey{"SPOOR_RUNTIME_SESSION_ID"};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
constexpr auto kSessionIdDefaultValue = [] {
  std::random_device seed{};
  std::default_random_engine engine{seed()};
  std::uniform_int_distribution<trace::SessionId> distribution{};
  return distribution(engine);
};
constexpr std::string_view kThreadEventBufferCapacityKey{
    "SPOOR_RUNTIME_THEAD_EVENT_BUFFER_CAPACITY"};
constexpr buffer::CircularBuffer<trace::Event>::SizeType
    kThreadEventBufferCapacityDefaultValue{10'000};
constexpr std::string_view kMaxReservedEventBufferSliceCapacityKey{
    "SPOOR_RUNTIME_MAX_RESERVED_EVENT_BUFFER_SLICE_CAPACITY"};
constexpr buffer::CircularBuffer<trace::Event>::SizeType
    kMaxReservedEventBufferSliceCapacityDefaultValue{1'000};
constexpr std::string_view kMaxDynamicEventBufferSliceCapacityKey{
    "SPOOR_RUNTIME_MAX_DYNAMIC_EVENT_BUFFER_SLICE_CAPACITY"};
constexpr buffer::CircularBuffer<trace::Event>::SizeType
    kMaxDynamicEventBufferSliceCapacityDefaultValue{1'000};
constexpr std::string_view kReservedEventPoolCapacityKey{
    "SPOOR_RUNTIME_RESERVED_EVENT_POOL_CAPACITY"};
constexpr buffer::CircularBuffer<trace::Event>::SizeType
    kReservedEventPoolCapacityDefaultValue{0};
constexpr std::string_view kDynamicEventPoolCapacityKey{
    "SPOOR_RUNTIME_DYNAMIC_EVENT_POOL_CAPACITY"};
constexpr buffer::CircularBuffer<
    trace::Event>::SizeType kDynamicEventPoolCapacityDefaultValue{
    std::numeric_limits<buffer::CircularBuffer<trace::Event>::SizeType>::max()};
constexpr std::string_view kDynamicEventSliceBorrowCasAttemptsKey{
    "SPOOR_RUNTIME_DYNAMIC_EVENT_SLICE_BORROW_CAS_ATTEMPTS"};
constexpr buffer::CircularBuffer<trace::Event>::SizeType
    kDynamicEventSliceBorrowCasAttemptsDefaultValue{1};
constexpr std::string_view kEventBufferRetentionDurationNanosecondsKey{
    "SPOOR_RUNTIME_EVENT_BUFFER_RETENTION_DURATION_NANOSECONDS"};
constexpr trace::DurationNanoseconds
    kEventBufferRetentionNanosecondsDefaultValue{0};
constexpr std::string_view kMaxFlushBufferToFileAttemptsKey{
    "SPOOR_RUNTIME_MAX_FLUSH_BUFFER_TO_FILE_ATTEMPTS"};
constexpr int32 kMaxFlushBufferToFileAttemptsDefaultValue{2};
constexpr std::string_view kFlushAllEventsKey{"SPOOR_RUNTIME_FLUSH_ALL_EVENTS"};
constexpr bool kFlushAllEventsDefaultValue{true};

struct Config {
  using SizeType = buffer::CircularBuffer<trace::Event>::SizeType;

  static auto FromEnv(const util::env::GetEnv& get_env = [](const char* key) {
    return std::getenv(key);
  }) -> Config;

  std::filesystem::path trace_file_path;
  CompressionStrategy compression_strategy;
  trace::SessionId session_id;
  SizeType thread_event_buffer_capacity;
  SizeType max_reserved_event_buffer_slice_capacity;
  SizeType max_dynamic_event_buffer_slice_capacity;
  SizeType reserved_event_pool_capacity;
  SizeType dynamic_event_pool_capacity;
  SizeType dynamic_event_slice_borrow_cas_attempts;
  trace::DurationNanoseconds event_buffer_retention_duration_nanoseconds;
  int32 max_flush_buffer_to_file_attempts;
  bool flush_all_events;
};

auto operator==(const Config& lhs, const Config& rhs) -> bool;

};  // namespace spoor::runtime::config
