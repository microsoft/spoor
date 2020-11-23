#pragma once

#include <cstdint>
#include <filesystem>
#include <system_error>

#include "util/result.h"

namespace util::file_system {

class FileSystem {
 public:
  FileSystem() = default;
  FileSystem(const FileSystem&) = default;
  FileSystem(FileSystem&&) noexcept = default;
  auto operator=(const FileSystem&) -> FileSystem& = default;
  auto operator=(FileSystem&&) noexcept -> FileSystem& = default;
  virtual ~FileSystem() = default;

  [[nodiscard]] virtual auto IsRegularFile(const std::filesystem::path& path)
      const -> util::result::Result<bool, std::error_code> = 0;
  [[nodiscard]] virtual auto FileSize(const std::filesystem::path& path) const
      -> util::result::Result<std::uintmax_t, std::error_code> = 0;
  [[nodiscard]] virtual auto Remove(const std::filesystem::path& path) const
      -> util::result::Result<util::result::None, std::error_code> = 0;
};

}  // namespace util::file_system
