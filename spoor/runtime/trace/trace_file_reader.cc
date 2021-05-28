// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace_file_reader.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <ios>
#include <vector>

#include "absl/base/internal/endian.h"
#include "absl/strings/str_cat.h"
#include "gsl/gsl"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor_factory.h"

namespace spoor::runtime::trace {

using Error = TraceReader::Error;

TraceFileReader::TraceFileReader(Options options)
    : options_{std::move(options)} {}

auto TraceFileReader::MatchesTraceFileConvention(
    const std::filesystem::path& file_path) const -> bool {
  const auto result = options_.file_system->IsRegularFile(file_path);
  const auto is_regular_file = result.OkOr(false);
  return is_regular_file &&
         file_path.extension() == absl::StrCat(".", kTraceFileExtension);
}

auto TraceFileReader::ReadHeader(const std::filesystem::path& file_path) const
    -> util::result::Result<Header, Error> {
  auto& file = *options_.file_reader;
  file.Open(file_path, std::ios::binary);
  if (!file.IsOpen()) return Error::kFailedToOpenFile;
  auto finally = gsl::finally([&file] { file.Close(); });
  const auto buffer = file.Read(sizeof(Header));
  if (buffer.size() != sizeof(Header)) return Error::kCorruptData;
  const auto header =
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      HeaderToHostByteOrder(*reinterpret_cast<const Header*>(buffer.data()));
  if (header.magic_number != kMagicNumber) return Error::kCorruptData;
  if (header.version != kTraceFileVersion) return Error::kUnknownVersion;
  return header;
}

auto TraceFileReader::Read(const std::filesystem::path& file_path) const
    -> util::result::Result<TraceFile, Error> {
  auto& file = *options_.file_reader;
  file.Open(file_path, std::ios::binary);
  if (!file.IsOpen()) return Error::kFailedToOpenFile;
  auto finally = gsl::finally([&file] { file.Close(); });
  const auto buffer = file.Read();
  if (buffer.size() < sizeof(Header)) return Error::kCorruptData;
  const auto header =
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      HeaderToHostByteOrder(*reinterpret_cast<const Header*>(buffer.data()));
  if (header.magic_number != kMagicNumber) return Error::kCorruptData;
  if (header.version != kTraceFileVersion) return Error::kUnknownVersion;
  const auto compressor = util::compression::MakeCompressor(
      header.compression_strategy, header.event_count * sizeof(Event));
  const auto compressed_data_begin =
      std::next(std::cbegin(buffer), sizeof(Header));
  const auto compressed_data_end = std::cend(buffer);
  const auto uncompressed_result = compressor->Uncompress(
      {&(*compressed_data_begin),
       gsl::narrow_cast<gsl::span<const char>::size_type>(
           std::distance(compressed_data_begin, compressed_data_end))});
  if (uncompressed_result.IsErr()) return Error::kCorruptData;
  const auto uncompressed_data = uncompressed_result.Ok();
  if (uncompressed_data.size() != header.event_count * sizeof(Event)) {
    return Error::kCorruptData;
  }
  std::vector<Event> events{};
  events.reserve(header.event_count);
  const auto* const uncompressed_buffer_events_begin =
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const Event*>(uncompressed_data.data());
  std::transform(
      uncompressed_buffer_events_begin,
      std::next(uncompressed_buffer_events_begin, header.event_count),
      std::back_inserter(events), [&](const auto event) {
        return EventToHostByteOrder(event, header.endianness);
      });
  return TraceFile{.header = header, .events = std::move(events)};
}

auto TraceFileReader::HeaderToHostByteOrder(const Header header) -> Header {
  if (header.endianness == kEndianness) return header;
  return {
      .magic_number = header.magic_number,
      .endianness = header.endianness,
      .version = absl::gbswap_32(header.version),
      .session_id = absl::gbswap_64(header.session_id),
      .process_id = static_cast<ProcessId>(
          absl::gbswap_64(static_cast<uint64>(header.process_id))),
      .thread_id = absl::gbswap_64(header.thread_id),
      .system_clock_timestamp = static_cast<TimestampNanoseconds>(
          absl::gbswap_64(static_cast<uint64>(header.system_clock_timestamp))),
      .steady_clock_timestamp = static_cast<TimestampNanoseconds>(
          absl::gbswap_64(static_cast<uint64>(header.steady_clock_timestamp))),
      .event_count = static_cast<EventCount>(
          absl::gbswap_32(static_cast<uint32>(header.event_count))),
      .padding = header.padding,
  };
}

auto TraceFileReader::EventToHostByteOrder(const Event event,
                                           const Endian data_endianness)
    -> Event {
  if (data_endianness == kEndianness) return event;
  return {
      .steady_clock_timestamp = static_cast<TimestampNanoseconds>(
          absl::gbswap_64(static_cast<uint64>(event.steady_clock_timestamp))),
      .payload_1 = absl::gbswap_64(event.payload_1),
      .type = static_cast<EventType>(
          absl::gbswap_64(static_cast<uint32>(event.type))),
      .payload_2 = absl::gbswap_32(event.payload_2),
  };
}

}  // namespace spoor::runtime::trace
