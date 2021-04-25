// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace_file_reader.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <regex>

#include "absl/base/internal/endian.h"
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
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* header = reinterpret_cast<Header*>(header_buffer.data());
  if (header->magic_number != kMagicNumber) {
    return Error::kMagicNumberDoesNotMatch;
  }
  if (kEndianness != header->endianness) {
    header->version = absl::gbswap_32(header->version);
    header->session_id = absl::gbswap_64(header->session_id);
    header->process_id = static_cast<ProcessId>(
        absl::gbswap_64(static_cast<uint64>(header->process_id)));
    header->thread_id = absl::gbswap_64(header->thread_id);
    header->system_clock_timestamp = static_cast<TimestampNanoseconds>(
        absl::gbswap_64(static_cast<uint64>(header->system_clock_timestamp)));
    header->steady_clock_timestamp = static_cast<TimestampNanoseconds>(
        absl::gbswap_64(static_cast<uint64>(header->steady_clock_timestamp)));
    header->event_count = static_cast<EventCount>(
        absl::gbswap_32(static_cast<uint32>(header->event_count)));
  }
  if (header->version != kTraceFileVersion) return Error::kUnknownVersion;
  return *header;
}

}  // namespace spoor::runtime::trace
