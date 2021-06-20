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
  MOCK_METHOD(  // NOLINT
      Result, Read, (const std::filesystem::path& file_path),
      (const, override));
};

}  // namespace spoor::instrumentation::filters::testing
