#pragma once

#include <cstdint>
#include <filesystem>
#include <system_error>

#include "util/file_system/file_system.h"
#include "util/result.h"

namespace util::file_system {

class LocalFileSystem final : public FileSystem {
 public:
  [[nodiscard]] auto IsRegularFile(const std::filesystem::path& path) const
      -> util::result::Result<bool, std::error_code> override;
  [[nodiscard]] auto FileSize(const std::filesystem::path& path) const
      -> util::result::Result<std::uintmax_t, std::error_code> override;
  [[nodiscard]] auto Remove(const std::filesystem::path& path) const
      -> util::result::Result<util::result::None, std::error_code> override;
};

}  // namespace util::file_system
