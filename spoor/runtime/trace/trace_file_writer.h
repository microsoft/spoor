// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "gsl/gsl"
#include "spoor/runtime/trace/trace.h"
#include "spoor/runtime/trace/trace_writer.h"
#include "util/compression/compressor.h"
#include "util/file_system/file_writer.h"

namespace spoor::runtime::trace {

class TraceFileWriter final : public TraceWriter {
 public:
  struct alignas(32) Options {
    std::unique_ptr<util::file_system::FileWriter> file_writer;
    util::compression::Strategy compression_strategy;
    util::compression::Compressor::SizeType initial_buffer_capacity;
  };

  explicit TraceFileWriter(Options options);
  auto Write(const std::filesystem::path& file_path, Header header,
             gsl::not_null<Events*> events) -> Result override;

 private:
  Options options_;
  std::unique_ptr<util::compression::Compressor> compressor_;
  std::vector<Event> buffer_;
};

}  // namespace spoor::runtime::trace
