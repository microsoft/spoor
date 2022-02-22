// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace_file_writer.h"

#include <algorithm>
#include <fstream>
#include <ios>
#include <string>
#include <utility>

#include "gsl/gsl"
#include "spoor/runtime/trace/trace.h"
#include "util/compression/compressor_factory.h"

namespace spoor::runtime::trace {

TraceFileWriter::TraceFileWriter(Options options)
    : options_{std::move(options)},
      compressor_{MakeCompressor(options_.compression_strategy,
                                 options_.initial_buffer_capacity)},
      buffer_{{}},
      created_directory_{!options_.create_directory} {
  buffer_.reserve(options_.initial_buffer_capacity);
}

auto TraceFileWriter::Write(const std::string& file_name, Header header,
                            gsl::not_null<Events*> events) -> Result {
  buffer_.resize(0);
  if (events->Empty()) return Result::Ok({});
  if (!created_directory_) {
    const auto result =
        options_.file_system->CreateDirectories(options_.directory);
    static_cast<void>(result);  // Handle the error later.
    created_directory_ = true;
  }

  const auto path = options_.directory / file_name;
  auto& file = *options_.file_writer;
  file.Open(path, std::ios::trunc | std::ios::binary);
  if (!file.IsOpen()) return Error::kFailedToOpenFile;
  auto finally = gsl::finally([&file] { file.Close(); });
  const auto chunks = events->ContiguousMemoryChunks();
  std::for_each(std::cbegin(chunks), std::cend(chunks),
                [this](const auto chunk) {
                  std::copy(std::cbegin(chunk), std::cend(chunk),
                            std::back_inserter(buffer_));
                });
  const auto buffer_size_bytes = buffer_.size() * sizeof(Event);
  const auto compression_result = compressor_->Compress(
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      {reinterpret_cast<const char*>(buffer_.data()), buffer_size_bytes});
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
