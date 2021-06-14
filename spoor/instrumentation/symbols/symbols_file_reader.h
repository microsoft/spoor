// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <memory>

#include "spoor/instrumentation/symbols/symbols_reader.h"
#include "util/file_system/file_reader.h"

namespace spoor::instrumentation::symbols {

class SymbolsFileReader final : public SymbolsReader {
 public:
  struct Options {
    std::unique_ptr<util::file_system::FileReader> file_reader;
  };

  explicit SymbolsFileReader(Options options);

  [[nodiscard]] auto Read(const std::filesystem::path& file_path) const
      -> Result override;

 private:
  Options options_;
};

}  // namespace spoor::instrumentation::symbols
