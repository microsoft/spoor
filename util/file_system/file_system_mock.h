#pragma once

#include <cstdint>
#include <filesystem>

#include "gmock/gmock.h"
#include "util/file_system/file_system.h"

namespace util::file_system::testing {

class FileSystemMock final : public FileSystem {
 public:
  MOCK_METHOD(  // NOLINT
      bool, IsRegularFile, (const std::filesystem::path& path),
      (const, noexcept, override));
  MOCK_METHOD(  // NOLINT
      std::uintmax_t, FileSize, (const std::filesystem::path& path),
      (const, noexcept, override));
  MOCK_METHOD(  // NOLINT
      bool, Remove, (const std::filesystem::path& path),
      (const, noexcept, override));
};

}  // namespace util::file_system::testing
