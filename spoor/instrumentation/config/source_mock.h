// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "spoor/instrumentation/config/config.h"
#include "spoor/instrumentation/config/source.h"

namespace spoor::instrumentation::config::testing {

class SourceMock final : public Source {
 public:
  SourceMock() = default;
  SourceMock(const SourceMock&) = delete;
  SourceMock(SourceMock&&) noexcept = delete;
  auto operator=(const SourceMock&) -> SourceMock& = delete;
  auto operator=(SourceMock&&) noexcept -> SourceMock& = delete;
  ~SourceMock() override = default;

  MOCK_METHOD((std::vector<ReadError>), Read, (), (override));  // NOLINT
  MOCK_METHOD(bool, IsRead, (), (const, override));             // NOLINT

  MOCK_METHOD(  // NOLINT
      (std::optional<bool>), GetEnableRuntime, (), (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<std::string>), GetFiltersFile, (), (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<bool>), GetForceBinaryOutput, (), (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<bool>), GetInitializeRuntime, (), (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<bool>), GetInjectInstrumentation, (), (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<std::string>), GetModuleId, (), (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<std::string>), GetOutputFile, (), (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<OutputLanguage>), GetOutputLanguage, (),
      (const, override));
  MOCK_METHOD(  // NOLINT
      (std::optional<std::string>), GetOutputSymbolsFile, (),
      (const, override));
};

}  // namespace spoor::instrumentation::config::testing
