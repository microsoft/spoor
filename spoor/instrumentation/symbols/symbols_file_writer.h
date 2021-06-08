// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <memory>

#include "gsl/gsl"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/instrumentation/symbols/symbols_writer.h"
#include "util/file_system/file_writer.h"

namespace spoor::instrumentation::symbols {

class SymbolsFileWriter final : public SymbolsWriter {
 public:
  struct Options {
    std::unique_ptr<util::file_system::FileWriter> file_writer;
  };

  explicit SymbolsFileWriter(Options options);

  auto Write(const std::filesystem::path& file_path, const Symbols& symbols)
      -> Result override;

 private:
  Options options_;
};

}  // namespace spoor::instrumentation::symbols
