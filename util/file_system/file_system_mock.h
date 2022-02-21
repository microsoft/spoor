// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <cstdint>
#include <filesystem>
#include <system_error>

#include "gmock/gmock.h"
#include "util/file_system/file_system.h"
#include "util/result.h"

namespace util::file_system::testing {

class FileSystemMock final : public FileSystem {
 public:
  MOCK_METHOD(  // NOLINT
      (util::result::Result<bool, std::error_code>), IsRegularFile,
      (const std::filesystem::path& path), (const, override));
  MOCK_METHOD(  // NOLINT
      (util::result::Result<bool, std::error_code>), IsDirectory,
      (const std::filesystem::path& path), (const, override));
  MOCK_METHOD(  // NOLINT
      (util::result::Result<bool, std::error_code>), CreateDirectories,
      (const std::filesystem::path& path), (const, override));
  MOCK_METHOD(  // NOLINT
      (util::result::Result<std::uintmax_t, std::error_code>), FileSize,
      (const std::filesystem::path& path), (const, override));
  MOCK_METHOD(  // NOLINT
      (util::result::Result<util::result::None, std::error_code>), Remove,
      (const std::filesystem::path& path), (const, override));
};

}  // namespace util::file_system::testing
