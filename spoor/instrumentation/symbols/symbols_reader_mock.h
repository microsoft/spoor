// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>

#include "gmock/gmock.h"
#include "spoor/instrumentation/symbols/symbols_reader.h"

namespace spoor::instrumentation::symbols::testing {

class SymbolsReaderMock final : public SymbolsReader {
 public:
  SymbolsReaderMock() = default;
  SymbolsReaderMock(const SymbolsReaderMock&) = delete;
  SymbolsReaderMock(SymbolsReaderMock&&) noexcept = delete;
  auto operator=(const SymbolsReaderMock&) -> SymbolsReaderMock& = delete;
  auto operator=(SymbolsReaderMock&&) noexcept -> SymbolsReaderMock& = delete;
  ~SymbolsReaderMock() override = default;

  MOCK_METHOD(  // NOLINT
      Result, Read, (const std::filesystem::path& file_path),
      (const, override));
};

}  // namespace spoor::instrumentation::symbols::testing
