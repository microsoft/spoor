// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>

#include "gmock/gmock.h"
#include "spoor/instrumentation/symbols/symbols_reader.h"

namespace spoor::instrumentation::symbols::testing {

class SymbolsReaderMock final : public SymbolsReader {
 public:
  MOCK_METHOD(  // NOLINT
      Result, Read, (const std::filesystem::path& file_path),
      (const, override));
};

}  // namespace spoor::instrumentation::symbols::testing
