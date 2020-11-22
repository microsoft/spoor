#include "util/file_system/local_file_system.h"

#include <cstdint>
#include <filesystem>

namespace util::file_system {

auto LocalFileSystem::IsRegularFile(
    const std::filesystem::path& path) const noexcept -> bool {
  return std::filesystem::is_regular_file(path);
}

auto LocalFileSystem::FileSize(const std::filesystem::path& path) const noexcept
    -> std::uintmax_t {
  return std::filesystem::file_size(path);
}

auto LocalFileSystem::Remove(const std::filesystem::path& path) const noexcept
    -> bool {
  return std::filesystem::remove(path);
}

}  // namespace util::file_system
