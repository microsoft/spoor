// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/file_system/local_file_system.h"

#include <cstdint>
#include <filesystem>
#include <system_error>

#include "util/result.h"

namespace util::file_system {

using util::result::None;

auto LocalFileSystem::IsRegularFile(const std::filesystem::path& path) const
    -> util::result::Result<bool, std::error_code> {
  std::error_code error_code{};
  const auto is_regular_file =
      std::filesystem::is_regular_file(path, error_code);
  const auto error = static_cast<bool>(error_code);
  if (error) return error_code;
  return is_regular_file;
}

auto LocalFileSystem::IsDirectory(const std::filesystem::path& path) const
    -> util::result::Result<bool, std::error_code> {
  std::error_code error_code{};
  const auto is_directory = std::filesystem::is_directory(path, error_code);
  const auto error = static_cast<bool>(error_code);
  if (error) return error_code;
  return is_directory;
}

auto LocalFileSystem::FileSize(const std::filesystem::path& path) const
    -> util::result::Result<std::uintmax_t, std::error_code> {
  std::error_code error_code{};
  const auto file_size = std::filesystem::file_size(path, error_code);
  const auto error = static_cast<bool>(error_code);
  if (error) return error_code;
  return file_size;
}

auto LocalFileSystem::Remove(const std::filesystem::path& path) const
    -> util::result::Result<None, std::error_code> {
  std::error_code error_code{};
  const auto success = std::filesystem::remove(path, error_code);
  const auto error = !success || static_cast<bool>(error_code);
  if (error) return error_code;
  return util::result::Result<None, std::error_code>::Ok({});
}

}  // namespace util::file_system
