// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <string>

#include "gsl/gsl"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/trace/trace.h"
#include "util/result.h"

namespace spoor::runtime::trace {

class TraceWriter {
 public:
  enum class Error {
    kFailedToOpenFile,
  };

  using Events = spoor::runtime::buffer::CircularSliceBuffer<Event>;
  using Result = util::result::Result<util::result::None, Error>;

  constexpr TraceWriter() = default;
  constexpr TraceWriter(const TraceWriter&) = default;
  constexpr TraceWriter(TraceWriter&&) noexcept = default;
  constexpr auto operator=(const TraceWriter&) -> TraceWriter& = default;
  constexpr auto operator=(TraceWriter&&) noexcept -> TraceWriter& = default;
  virtual ~TraceWriter() = default;

  // Passing `const std::string&` instead of `std::string_view` for gMock
  // compatibility.
  virtual auto Write(const std::string& file_name, Header header,
                     gsl::not_null<Events*> events) -> Result = 0;
};

}  // namespace spoor::runtime::trace
