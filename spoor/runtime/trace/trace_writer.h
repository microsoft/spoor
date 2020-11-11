#pragma once

#include <filesystem>

#include "absl/strings/str_cat.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/trace/trace.h"
#include "util/result.h"

namespace spoor::runtime::trace {

class TraceWriter {
 public:
  using Events = spoor::runtime::buffer::CircularSliceBuffer<Event>;
  using Result = util::result::Result<util::result::None, util::result::None>;

  constexpr TraceWriter() = default;
  constexpr TraceWriter(const TraceWriter&) = default;
  constexpr TraceWriter(TraceWriter&&) = default;
  constexpr auto operator=(const TraceWriter&) -> TraceWriter& = default;
  constexpr auto operator=(TraceWriter&&) -> TraceWriter& = default;
  virtual ~TraceWriter() = default;

  virtual auto Write(const std::filesystem::path& file, Header header,
                     Events* events, Footer footer) const -> Result = 0;
};

class TraceFileWriter final : public TraceWriter {
 public:
  auto Write(const std::filesystem::path& file, Header header, Events* events,
             Footer footer) const -> Result override;
};

}  // namespace spoor::runtime::trace
