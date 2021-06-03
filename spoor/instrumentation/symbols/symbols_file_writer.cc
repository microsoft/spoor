// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/symbols/symbols_file_writer.h"

#include <filesystem>

#include "gsl/gsl"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/instrumentation/symbols/symbols_writer.h"

namespace spoor::instrumentation::symbols {

SymbolsFileWriter::SymbolsFileWriter(Options options)
    : options_{std::move(options)} {}

auto SymbolsFileWriter::Write(const std::filesystem::path& file_path,
                              const Symbols& symbols) -> Result {
  auto& file = *options_.file_writer;
  file.Open(file_path, std::ios::binary);
  if (!file.IsOpen()) return Error::kFailedToOpenFile;
  auto finally = gsl::finally([&file] { file.Close(); });
  const auto success = symbols.SerializeToOstream(&file.Ostream());
  if (success) return Result::Ok({});
  return Error::kSerializationError;
}

}  // namespace spoor::instrumentation::symbols
