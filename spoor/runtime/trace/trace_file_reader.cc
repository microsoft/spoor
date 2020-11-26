// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace_file_reader.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <regex>

#include "spoor/runtime/trace/trace.h"

namespace spoor::runtime::trace {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::regex kTraceFileNamePattern{
    R"([0-9a-f]{16}-[0-9a-f]{16}-[0-9a-f]{16}\.spoor)"};

TraceFileReader::TraceFileReader(Options options)
    : options_{std::move(options)} {}

auto TraceFileReader::MatchesTraceFileConvention(
    const std::filesystem::path& file) const -> bool {
  const auto result = options_.file_system->IsRegularFile(file);
  const auto is_regular_file = result.OkOr(false);
  return is_regular_file &&
         std::regex_match(file.filename().string(), kTraceFileNamePattern);
}

auto TraceFileReader::ReadHeader(const std::filesystem::path& file) const
    -> Result {
  std::ifstream file_stream{file};
  if (!file_stream.is_open()) return Error::kFailedToOpenFile;
  std::array<char, sizeof(Header)> header_buffer{};
  file_stream.read(header_buffer.data(), header_buffer.size());
  const auto header = Deserialize(header_buffer);
  if (header.version != kTraceFileVersion) return Error::kUnknownVersion;
  return header;
}

}  // namespace spoor::runtime::trace
