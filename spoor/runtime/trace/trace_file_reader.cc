// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace_file_reader.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <regex>
#include <iostream>  // TODO

#include "absl/base/internal/endian.h"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor.h"
#include "util/compression/compressor_factory.h"

namespace spoor::runtime::trace {

using spoor::runtime::trace::Event;
using spoor::runtime::trace::Header;
using spoor::runtime::trace::TraceFile;
using util::compression::MakeCompressor;
using CompressionStrategy = util::compression::Strategy;

TraceFileReader::TraceFileReader(Options options)
    : options_{std::move(options)} {}

// auto TraceFileReader::HasTraceFileExtension(
//     const std::filesystem::path& path) const -> bool {
//   return path.extension() == kTraceFileExtension;
// }

auto TraceFileReader::Read(const std::filesystem::path& path,
                           const bool read_events) -> Result {
  const auto file_size = std::filesystem::file_size(path);
  if (file_size < sizeof(Header)) return Error::kMalformedFile;
  std::ifstream file{};  // TODO custom file reader
  file.open(path);
  if (!file.is_open()) return Error::kFailedToOpenFile;
  std::array<char, sizeof(Header)> header_buffer{};
  file.read(header_buffer.data(), header_buffer.size());
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto header = *reinterpret_cast<Header*>(header_buffer.data());
  if (header.magic_number != kMagicNumber) {
    return Error::kMismatchedMagicNumber;
  }
  if (header.endianness != kEndianness) {
    header.version = absl::gbswap_32(header.version);
    header.session_id = absl::gbswap_64(header.session_id);
    header.process_id = absl::gbswap_64(header.process_id);
    header.thread_id = absl::gbswap_64(header.thread_id);
    header.system_clock_timestamp =
        absl::gbswap_64(header.system_clock_timestamp);
    header.steady_clock_timestamp =
        absl::gbswap_64(header.steady_clock_timestamp);
    header.event_count = absl::gbswap_32(header.event_count);
  }
  if (header.version != kTraceFileVersion) return Error::kUnknownVersion;

  std::vector<Event> events{};
  if (read_events) {
    file.seekg(sizeof(Header));
    const auto compressed_events_size = file_size - sizeof(Header);
    std::string compressed_buffer(compressed_events_size, '\0');
    file.read(compressed_buffer.data(), compressed_buffer.size());
    const auto events_size_bytes = header.event_count * sizeof(Event);
    auto compressor = CompressorForStrategy(
        header.compression_strategy, events_size_bytes);
    switch (header.compression_strategy) {
      case CompressionStrategy::kNone: {
        std::cout << "Compression: none\n";
        break;
      }
      case CompressionStrategy::kSnappy: {
        std::cout << "Compression: snappy\n";
        break;
      }
    }
    auto uncompress_result = compressor->Uncompress(compressed_buffer);
    if (uncompress_result.IsErr()) return Error::kUncompressError;
    auto uncompressed_buffer = uncompress_result.Ok();
    if (uncompressed_buffer.size_bytes() != events_size_bytes) {
      return Error::kMalformedFile;
    }
    events.reserve(header.event_count);
    const auto* uncompressed_events =
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const Event*>(uncompressed_buffer.data());
    for (auto event_index{0}; event_index < header.event_count;
         ++event_index) {
      auto event = *std::next(uncompressed_events, event_index);
      if (header.endianness != kEndianness) {
        event.steady_clock_timestamp =
            absl::gbswap_64(event.steady_clock_timestamp);
        event.payload_1 = absl::gbswap_64(event.payload_1);
        event.type = absl::gbswap_32(event.type);
        event.payload_2 = absl::gbswap_32(event.payload_2);
      }
      events.emplace_back(event);
    }
  }

  return TraceFile{.session_id = header.session_id,
                   .process_id = header.process_id,
                   .thread_id = header.thread_id,
                   .system_clock_timestamp = header.system_clock_timestamp,
                   .steady_clock_timestamp = header.steady_clock_timestamp,
                   .events = events};
};

auto TraceFileReader::CompressorForStrategy(
    util::compression::Strategy compression_strategy,
    util::compression::Compressor::SizeType initial_capacity)
    -> gsl::not_null<util::compression::Compressor*> {
  auto compressor_iterator = compressors_.find(compression_strategy);
  if (compressor_iterator == std::cend(compressors_)) {
    auto compressor = MakeCompressor(compression_strategy, initial_capacity);
    auto* compressor_ptr = compressor.get();
    compressors_[compression_strategy] = std::move(compressor);
    return compressor_ptr;
  }
  auto& compressor = compressor_iterator->second;
  return compressor.get();
}

}  // namespace spoor::runtime::trace
