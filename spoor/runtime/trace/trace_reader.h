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
    kMalformedFile,
    kMismatchedMagicNumber,
    kUnknownVersion,
    kUncompressError,
  };

  using Result = util::result::Result<TraceFile, Error>;

  constexpr TraceReader() = default;
  constexpr TraceReader(const TraceReader&) = default;
  constexpr TraceReader(TraceReader&&) = default;
  constexpr auto operator=(const TraceReader&) -> TraceReader& = default;
  constexpr auto operator=(TraceReader &&) -> TraceReader& = default;
  virtual ~TraceReader() = default;

  // [[nodiscard]] virtual auto HasTraceFileExtension(
  //     const std::filesystem::path& path) const -> bool = 0;
  // [[nodiscard]] virtual auto IsTraceFile(
  //     const std::filesystem::path& path) const -> bool = 0;
  [[nodiscard]] virtual auto Read(const std::filesystem::path& path,
                                  bool read_events) -> Result = 0;
};

}  // namespace spoor::runtime::trace
