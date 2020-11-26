// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace_file_writer.h"

#include <filesystem>
#include <fstream>
#include <ios>

#include "spoor/runtime/trace/trace.h"

namespace spoor::runtime::trace {

auto TraceFileWriter::Write(const std::filesystem::path& file_path,
                            const Header header, Events* events,
                            const Footer footer) const -> Result {
  std::ofstream file{file_path, std::ios::trunc | std::ios::binary};
  if (!file.is_open()) return Error::kFailedToOpenFile;
  const auto serialized_header = Serialize(header);
  file.write(serialized_header.data(), serialized_header.size());
  for (auto& chunk : events->ContiguousMemoryChunks()) {
    for (const auto event : chunk) {
      const auto serialized_event = Serialize(event);
      file.write(serialized_event.data(), serialized_event.size());
    }
  }
  const auto serialized_footer = Serialize(footer);
  file.write(serialized_footer.data(), serialized_footer.size());
  return Result::Ok({});
}

}  // namespace spoor::runtime::trace
