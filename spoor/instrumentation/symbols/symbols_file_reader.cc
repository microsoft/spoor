// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/symbols/symbols_file_reader.h"

#include <filesystem>
#include <fstream>
#include <utility>

#include "gsl/gsl"
#include "spoor/instrumentation/symbols/symbols.pb.h"

namespace spoor::instrumentation::symbols {

SymbolsFileReader::SymbolsFileReader(Options options)
    : options_{std::move(options)} {}

auto SymbolsFileReader::Read(const std::filesystem::path& file_path) const
    -> Result {
  auto& file = *options_.file_reader;
  file.Open(file_path, std::ios::binary);
  if (!file.IsOpen()) return Error::kFailedToOpenFile;
  auto finally = gsl::finally([&file] { file.Close(); });
  Symbols symbols{};
  const auto success = symbols.ParseFromIstream(&file.Istream());
  if (success) return symbols;
  return Error::kCorruptData;
}

}  // namespace spoor::instrumentation::symbols
