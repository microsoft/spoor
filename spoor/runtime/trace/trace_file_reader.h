// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <memory>

#include "spoor/runtime/trace/trace_reader.h"
#include "util/file_system/file_reader.h"
#include "util/file_system/file_system.h"

namespace spoor::runtime::trace {

class TraceFileReader final : public TraceReader {
 public:
  struct Options {
    std::unique_ptr<util::file_system::FileSystem> file_system;
    std::unique_ptr<util::file_system::FileReader> file_reader;
  };

  explicit TraceFileReader(Options options);

  [[nodiscard]] auto MatchesTraceFileConvention(
      const std::filesystem::path& file_path) const -> bool override;
  [[nodiscard]] auto ReadHeader(const std::filesystem::path& file_path) const
      -> util::result::Result<Header, Error> override;
  [[nodiscard]] auto Read(const std::filesystem::path& file_path) const
      -> util::result::Result<TraceFile, Error> override;

 private:
  Options options_;

  [[nodiscard]] static auto HeaderToHostByteOrder(Header header) -> Header;
  [[nodiscard]] static auto EventToHostByteOrder(Event event,
                                                 Endian data_endianness)
      -> Event;
};

}  // namespace spoor::runtime::trace
