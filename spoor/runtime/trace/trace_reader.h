// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>

#include "spoor/runtime/trace/trace.h"
#include "util/file_system/file_system.h"
#include "util/result.h"

namespace spoor::runtime::trace {

class TraceReader {
 public:
  enum class Error {
    kFailedToOpenFile,
    kUnknownVersion,
    kCorruptData,
  };

  constexpr TraceReader() = default;
  constexpr TraceReader(const TraceReader&) = default;
  constexpr TraceReader(TraceReader&&) = default;
  constexpr auto operator=(const TraceReader&) -> TraceReader& = default;
  constexpr auto operator=(TraceReader&&) -> TraceReader& = default;
  virtual ~TraceReader() = default;

  [[nodiscard]] virtual auto MatchesTraceFileConvention(
      const std::filesystem::path& file_path) const -> bool = 0;
  [[nodiscard]] virtual auto ReadHeader(const std::filesystem::path& file_path)
      const -> util::result::Result<Header, Error> = 0;
  [[nodiscard]] virtual auto Read(const std::filesystem::path& file_path) const
      -> util::result::Result<TraceFile, Error> = 0;
};

}  // namespace spoor::runtime::trace
