// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <memory>
#include <type_traits>

#include "spoor/runtime/buffer/circular_buffer.h"
#include "spoor/runtime/config/source.h"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor.h"
#include "util/env/env.h"
#include "util/file_system/util.h"

namespace spoor::runtime::config {

struct Config {
  using SizeType = buffer::CircularBuffer<trace::Event>::SizeType;
  static_assert(std::is_same_v<SizeType, Source::SizeType>);

  std::filesystem::path trace_file_path;
  util::compression::Strategy compression_strategy;
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

  static auto Default() -> Config;
  static auto FromSourcesOrDefault(
      std::vector<std::unique_ptr<Source>>&& sources,
      const Config& default_config,
      const util::file_system::PathExpansionOptions& path_expansion_options)
      -> Config;
};

auto operator==(const Config& lhs, const Config& rhs) -> bool;

};  // namespace spoor::runtime::config
