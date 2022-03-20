// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>

#include "gmock/gmock.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_reader.h"
#include "util/result.h"

namespace spoor::runtime::trace::testing {

class TraceReaderMock final : public TraceReader {
 public:
  TraceReaderMock() = default;
  TraceReaderMock(const TraceReaderMock&) = delete;
  TraceReaderMock(TraceReaderMock&&) noexcept = delete;
  auto operator=(const TraceReaderMock&) -> TraceReaderMock& = delete;
  auto operator=(TraceReaderMock&&) noexcept -> TraceReaderMock& = delete;
  ~TraceReaderMock() override = default;

  MOCK_METHOD(  // NOLINT
      bool, MatchesTraceFileConvention,
      (const std::filesystem::path& file_path), (const, override));
  MOCK_METHOD(  // NOLINT
      (util::result::Result<Header, Error>), ReadHeader,
      (const std::filesystem::path& file_path), (const, override));
  MOCK_METHOD(  // NOLINT
      (util::result::Result<TraceFile, Error>), Read,
      (const std::filesystem::path& file_path), (const, override));
};

}  // namespace spoor::runtime::trace::testing
