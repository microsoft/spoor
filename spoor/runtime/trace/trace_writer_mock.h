// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <string>

#include "gmock/gmock.h"
#include "gsl/gsl"
#include "spoor/runtime/trace/trace_writer.h"

namespace spoor::runtime::trace::testing {

class TraceWriterMock final : public TraceWriter {
 public:
  TraceWriterMock() = default;
  TraceWriterMock(const TraceWriterMock&) = delete;
  TraceWriterMock(TraceWriterMock&&) noexcept = delete;
  auto operator=(const TraceWriterMock&) -> TraceWriterMock& = delete;
  auto operator=(TraceWriterMock&&) noexcept -> TraceWriterMock& = delete;
  ~TraceWriterMock() override = default;

  MOCK_METHOD(  // NOLINT
      Result, Write,
      (const std::string& file_name, Header header,
       gsl::not_null<Events*> events),
      (override));
};

}  // namespace spoor::runtime::trace::testing
