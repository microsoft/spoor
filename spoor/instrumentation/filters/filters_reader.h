// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "spoor/instrumentation/filters/filter.h"
#include "util/result.h"

namespace spoor::instrumentation::filters {

class FiltersReader {
 public:
  struct Error {
    enum class Type {
      kFailedToOpenFile,
      kMalformedFile,
      kMalformedNode,
      kUnknownNode,
    };

    Type type;
    std::string message;
  };

  using Result = util::result::Result<std::vector<Filter>, Error>;

  constexpr FiltersReader() = default;
  constexpr FiltersReader(const FiltersReader&) = default;
  constexpr FiltersReader(FiltersReader&&) = default;
  constexpr auto operator=(const FiltersReader&) -> FiltersReader& = default;
  constexpr auto operator=(FiltersReader&&) -> FiltersReader& = default;
  virtual ~FiltersReader() = default;

  [[nodiscard]] virtual auto Read(const std::filesystem::path& file_path) const
      -> Result = 0;
};

}  // namespace spoor::instrumentation::filters
