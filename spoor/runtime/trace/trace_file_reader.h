// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <memory>
#include <unordered_map>

#include "gsl/gsl"
#include "spoor/runtime/trace/trace_reader.h"
// #include "util/file_system/file_reader.h"
#include "util/compression/compressor.h"

namespace spoor::runtime::trace {

class TraceFileReader final : public TraceReader {
 public:
  struct Options {
    // gsl::not_null<util::file_system::FileReader*> file_reader;
    util::compression::Compressor::SizeType initial_buffer_capacity;
  };

  explicit TraceFileReader(Options options);

  // [[nodiscard]] auto HasTraceFileExtension(
  //     const std::filesystem::path& path) const -> bool override;
  // [[nodiscard]] auto IsTraceFile(const std::filesystem::path& path) const
  //     -> bool override;
  [[nodiscard]] auto Read(const std::filesystem::path& path, bool read_events)
      -> Result override;

 private:
  Options options_;
  std::unordered_map<util::compression::Strategy,
                     std::unique_ptr<util::compression::Compressor>>
      compressors_;

  [[nodiscard]] auto CompressorForStrategy(
      util::compression::Strategy compression_strategy,
      util::compression::Compressor::SizeType initial_capacity)
      -> gsl::not_null<util::compression::Compressor*>;
};

}  // namespace spoor::runtime::trace
