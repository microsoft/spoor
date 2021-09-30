// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// Licensed under the MIT License.

#pragma once

#include <filesystem>

#include "gmock/gmock.h"
#include "spoor/instrumentation/filters/filters_reader.h"

namespace spoor::instrumentation::filters::testing {

class FiltersReaderMock final : public FiltersReader {
 public:
  FiltersReaderMock() = default;
  FiltersReaderMock(const FiltersReaderMock&) = delete;
  FiltersReaderMock(FiltersReaderMock&&) noexcept = delete;
  auto operator=(const FiltersReaderMock&) -> FiltersReaderMock& = delete;
  auto operator=(FiltersReaderMock&&) noexcept -> FiltersReaderMock& = delete;
  ~FiltersReaderMock() override = default;

  MOCK_METHOD(  // NOLINT
      Result, Read, (const std::filesystem::path& file_path),
      (const, override));
};

}  // namespace spoor::instrumentation::filters::testing
