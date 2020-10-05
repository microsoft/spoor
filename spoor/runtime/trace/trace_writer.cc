#include "spoor/runtime/trace/trace_writer.h"

#include <filesystem>
#include <fstream>
#include <ios>

#include "spoor/runtime/trace/trace.h"

namespace spoor::runtime::trace {

auto TraceFileWriter::Write(const std::filesystem::path& file_path,
                            const Header& header, Events& events,
                            const Footer& footer) const -> Result {
  std::ofstream file{file_path,
                     std::ios::out | std::ios::trunc | std::ios::binary};
  // TODO network byte order
  if (!file.is_open()) return Result::Err({});
  file.write(reinterpret_cast<const char*>(&header), sizeof(header));
  for (auto& chunk : events.ContiguousMemoryChunks()) {
    file.write(reinterpret_cast<const char*>(chunk.data()), chunk.size_bytes());
  }
  file.write(reinterpret_cast<const char*>(&footer), sizeof(footer));
  return Result::Ok({});
}

}  // namespace spoor::runtime::trace
