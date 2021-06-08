// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <ios>
#include <istream>

#include "gmock/gmock.h"
#include "util/file_system/file_reader.h"

namespace util::file_system::testing {

class FileReaderMock final : public FileReader {
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
      (std::string), Read, (), (override));
  MOCK_METHOD(  // NOLINT
      (std::string), Read, (std::streamsize max_bytes), (override));
  MOCK_METHOD(  // NOLINT
      (std::istream&), Istream, (), (override));
  MOCK_METHOD(  // NOLINT
      (void), Close, (), (override));
};

}  // namespace util::file_system::testing
