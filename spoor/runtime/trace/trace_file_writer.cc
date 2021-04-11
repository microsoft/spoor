// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace_file_writer.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <ios>
#include <iostream>  // TODO

#include "gsl/gsl"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor_factory.h"

namespace spoor::runtime::trace {

TraceFileWriter::TraceFileWriter(Options options)
    : options_{std::move(options)},
      compressor_{MakeCompressor(options_.compression_strategy,
                                 options_.initial_buffer_capacity)} {
  buffer_.reserve(options_.initial_buffer_capacity);
}

auto TraceFileWriter::Write(const std::filesystem::path& file_path,
                            Header header, gsl::not_null<Events*> events)
    -> Result {
  buffer_.resize(0);
  if (events->Empty()) return Result::Ok({});
  auto& file = *options_.file_writer;
  file.Open(file_path, std::ios::trunc | std::ios::binary);
  if (!file.IsOpen()) return Error::kFailedToOpenFile;
  auto finally = gsl::finally([&file] { file.Close(); });
  const auto chunks = events->ContiguousMemoryChunks();
  std::for_each(std::cbegin(chunks), std::cend(chunks),
                [this](const auto chunk) {
                  std::copy(std::cbegin(chunk), std::cend(chunk),
                            std::back_inserter(buffer_));
                });
  const auto buffer_size_bytes = buffer_.size() * sizeof(Event);
  // TODO
  switch (compressor_->Strategy()) {
    case CompressionStrategy::kNone: {
      std::cout << "compression strategy: none\n";
      break;
    }
    case CompressionStrategy::kSnappy: {
      std::cout << "compression strategy: snappy\n";
      break;
    }
  }
  const auto compression_result = compressor_->Compress(
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      {reinterpret_cast<const char*>(buffer_.data()), buffer_size_bytes});
  if (compression_result.IsErr()) {
    std::cout << "compression error\n";
  }
  if (buffer_size_bytes <= compression_result.Ok().size_bytes()) {
    std::cout << "inefficient compression\n";
  }
  std::cout << "uncompressed size: " << buffer_size_bytes << "\n";
  std::cout << "compressed size: " << compression_result.Ok().size_bytes() << "\n";
  if (compression_result.IsErr() ||
      buffer_size_bytes <= compression_result.Ok().size_bytes()) {
    header.compression_strategy = util::compression::Strategy::kNone;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    file.Write({reinterpret_cast<const char*>(&header), sizeof(header)});
    file.Write(  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        {reinterpret_cast<const char*>(buffer_.data()), buffer_size_bytes});
  } else {
    const auto compressed = compression_result.Ok();
    header.compression_strategy = compressor_->Strategy();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    file.Write({reinterpret_cast<const char*>(&header), sizeof(header)});
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    file.Write({reinterpret_cast<const char*>(compressed.data()),
                compressed.size_bytes()});
  }
  return Result::Ok({});
}

}  // namespace spoor::runtime::trace
