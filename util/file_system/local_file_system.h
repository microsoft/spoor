#pragma once

#include <cstdint>
#include <filesystem>

#include "util/file_system/file_system.h"

namespace util::file_system {

class LocalFileSystem final : public FileSystem {
 public:
  [[nodiscard]] auto IsRegularFile(
      const std::filesystem::path& path) const noexcept -> bool override;
  [[nodiscard]] auto FileSize(const std::filesystem::path& path) const noexcept
      -> std::uintmax_t override;
  [[nodiscard]] auto Remove(const std::filesystem::path& path) const noexcept
      -> bool override;
};

}  // namespace util::file_system
