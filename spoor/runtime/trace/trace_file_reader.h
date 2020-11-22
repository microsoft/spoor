#pragma once

#include <filesystem>

#include "gsl/gsl"
#include "spoor/runtime/trace/trace_reader.h"
#include "util/file_system/file_system.h"

namespace spoor::runtime::trace {

class TraceFileReader final : public TraceReader {
 public:
  struct Options {
    gsl::not_null<util::file_system::FileSystem*> file_system;
  };

  explicit TraceFileReader(Options options);

  [[nodiscard]] auto MatchesTraceFileConvention(
      const std::filesystem::path& file) const -> bool override;
  [[nodiscard]] auto ReadHeader(const std::filesystem::path& file) const
      -> Result override;

 private:
  Options options_;
};

}  // namespace spoor::runtime::trace
