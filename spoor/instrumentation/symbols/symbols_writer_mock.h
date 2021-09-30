// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>

#include "gmock/gmock.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/instrumentation/symbols/symbols_writer.h"

namespace spoor::instrumentation::symbols::testing {

class SymbolsWriterMock final : public SymbolsWriter {
 public:
  SymbolsWriterMock() = default;
  SymbolsWriterMock(const SymbolsWriterMock&) = delete;
  SymbolsWriterMock(SymbolsWriterMock&&) noexcept = delete;
  auto operator=(const SymbolsWriterMock&) -> SymbolsWriterMock& = delete;
  auto operator=(SymbolsWriterMock&&) noexcept -> SymbolsWriterMock& = delete;
  ~SymbolsWriterMock() override = default;

  MOCK_METHOD(  // NOLINT
      Result, Write,
      (const std::filesystem::path& file_path, const Symbols& symbols),
      (override));
};

}  // namespace spoor::instrumentation::symbols::testing
