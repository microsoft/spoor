// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <filesystem>

#include "gmock/gmock.h"
#include "spoor/runtime/trace/trace_writer.h"

namespace spoor::runtime::trace::testing {

class TraceWriterMock final : public TraceWriter {
 public:
  MOCK_METHOD(  // NOLINT
      Result, Write,
      (const std::filesystem::path& file, Header header, Events* events),
      (const, override));
};

}  // namespace spoor::runtime::trace::testing
