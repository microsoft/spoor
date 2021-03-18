// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace_file_writer.h"

#include <filesystem>
#include <fstream>
#include <ios>

#include "spoor/runtime/trace/trace.h"

namespace spoor::runtime::trace {

auto TraceFileWriter::Write(const std::filesystem::path& file_path,
                            const Header header, Events* events) const
    -> Result {
  std::ofstream file{file_path, std::ios::trunc | std::ios::binary};
  if (!file.is_open()) return Error::kFailedToOpenFile;
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  file.write(reinterpret_cast<const char*>(&header), sizeof(header));
  for (auto& chunk : events->ContiguousMemoryChunks()) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    file.write(reinterpret_cast<const char*>(chunk.data()), chunk.size_bytes());
  }
  return Result::Ok({});
}

}  // namespace spoor::runtime::trace
