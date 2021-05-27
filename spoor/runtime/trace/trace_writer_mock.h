// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <filesystem>

#include "gmock/gmock.h"
#include "gsl/gsl"
#include "spoor/runtime/trace/trace_writer.h"

namespace spoor::runtime::trace::testing {

class TraceWriterMock final : public TraceWriter {
 public:
  MOCK_METHOD(  // NOLINT
      Result, Write,
      (const std::filesystem::path& file_path, Header header,
       gsl::not_null<Events*> events),
      (override));
};

}  // namespace spoor::runtime::trace::testing
