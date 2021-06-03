// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <ios>
#include <ostream>

#include "gmock/gmock.h"
#include "util/file_system/file_writer.h"

namespace util::file_system::testing {

class FileWriterMock final : public FileWriter {
 public:
  MOCK_METHOD(  // NOLINT
      (void), Open, (const std::filesystem::path& path), (override));
  MOCK_METHOD(  // NOLINT
      (void), Open,
      (const std::filesystem::path& path, std::ios_base::openmode mode),
      (override));
  MOCK_METHOD(  // NOLINT
      (bool), IsOpen, (), (const, override));
  MOCK_METHOD(  // NOLINT
      (void), Write, (gsl::span<const char> data), (override));
  MOCK_METHOD(  // NOLINT
      (std::ostream&), Ostream, (), (override));
  MOCK_METHOD(  // NOLINT
      (void), Close, (), (override));
};

}  // namespace util::file_system::testing
