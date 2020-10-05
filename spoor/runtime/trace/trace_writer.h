#pragma once

#include <filesystem>

#include "absl/strings/str_cat.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "spoor/runtime/trace/trace.h"
#include "util/result.h"

namespace spoor::runtime::trace {

struct Info {
  std::filesystem::path trace_file_path;
  TimestampNanoseconds flush_timestamp;
  TraceFileVersion version;
  SessionId session_id;
  ProcessId process_id;
  ThreadId thread_id;
};

class TraceWriter {
 public:
  using Events = spoor::runtime::buffer::CircularSliceBuffer<Event>;
  using Result = util::result::Result<util::result::None, util::result::None>;

  TraceWriter() = default;
  TraceWriter(const TraceWriter&) = default;
  TraceWriter(TraceWriter&&) = default;
  auto operator=(const TraceWriter&) -> TraceWriter& = default;
  auto operator=(TraceWriter &&) -> TraceWriter& = default;
  virtual ~TraceWriter() = default;

  virtual auto Write(const std::filesystem::path& file, const Header& header,
                     Events& events, const Footer& footer) const -> Result = 0;
};

class TraceFileWriter final : public TraceWriter {
 public:
  auto Write(const std::filesystem::path& file, const Header& header,
             Events& events, const Footer& footer) const -> Result override;
};

}  // namespace spoor::runtime::trace
