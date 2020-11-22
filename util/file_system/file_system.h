#pragma once

#include <cstdint>
#include <filesystem>

namespace util::file_system {

class FileSystem {
 public:
  FileSystem() = default;
  FileSystem(const FileSystem&) = default;
  FileSystem(FileSystem&&) noexcept = default;
  auto operator=(const FileSystem&) -> FileSystem& = default;
  auto operator=(FileSystem&&) noexcept -> FileSystem& = default;
  virtual ~FileSystem() = default;

  [[nodiscard]] virtual auto IsRegularFile(
      const std::filesystem::path& path) const noexcept -> bool = 0;
  [[nodiscard]] virtual auto FileSize(
      const std::filesystem::path& path) const noexcept -> std::uintmax_t = 0;
  [[nodiscard]] virtual auto Remove(
      const std::filesystem::path& path) const noexcept -> bool = 0;
};

}  // namespace util::file_system
