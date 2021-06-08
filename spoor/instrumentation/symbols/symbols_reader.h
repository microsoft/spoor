// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>

#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "util/result.h"

namespace spoor::instrumentation::symbols {

class SymbolsReader {
 public:
  enum class Error {
    kFailedToOpenFile,
    kCorruptData,
  };

  using Result = util::result::Result<Symbols, Error>;

  constexpr SymbolsReader() = default;
  constexpr SymbolsReader(const SymbolsReader&) = default;
  constexpr SymbolsReader(SymbolsReader&&) = default;
  constexpr auto operator=(const SymbolsReader&) -> SymbolsReader& = default;
  constexpr auto operator=(SymbolsReader&&) -> SymbolsReader& = default;
  virtual ~SymbolsReader() = default;

  [[nodiscard]] virtual auto Read(const std::filesystem::path& file_path) const
      -> Result = 0;
};

}  // namespace spoor::instrumentation::symbols
