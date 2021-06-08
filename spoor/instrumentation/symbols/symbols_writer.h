// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>

#include "gsl/gsl"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "util/result.h"

namespace spoor::instrumentation::symbols {

class SymbolsWriter {
 public:
  enum class Error {
    kFailedToOpenFile,
    kSerializationError,
  };

  using Result = util::result::Result<util::result::None, Error>;

  constexpr SymbolsWriter() = default;
  constexpr SymbolsWriter(const SymbolsWriter&) = default;
  constexpr SymbolsWriter(SymbolsWriter&&) = default;
  constexpr auto operator=(const SymbolsWriter&) -> SymbolsWriter& = default;
  constexpr auto operator=(SymbolsWriter&&) -> SymbolsWriter& = default;
  virtual ~SymbolsWriter() = default;

  virtual auto Write(const std::filesystem::path& file_path,
                     const Symbols& symbols) -> Result = 0;
};

}  // namespace spoor::instrumentation::symbols
