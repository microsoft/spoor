#pragma once

#include <filesystem>

#include "gmock/gmock.h"
#include "spoor/runtime/trace/trace_reader.h"

namespace spoor::runtime::trace::testing {

class TraceReaderMock final : public TraceReader {
 public:
  MOCK_METHOD(  // NOLINT
      bool, MatchesTraceFileConvention, (const std::filesystem::path& file),
      (const, override));
  MOCK_METHOD(  // NOLINT
      Result, ReadHeader, (const std::filesystem::path& file),
      (const, override));
};

}  // namespace spoor::runtime::trace::testing
