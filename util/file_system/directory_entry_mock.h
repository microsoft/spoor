#pragma once

#include <filesystem>

namespace util::file_system::testing {

class DirectoryEntryMock {
 public:
  explicit DirectoryEntryMock(std::filesystem::path path);

  // Match the signature of `std::filesystem::directory_entry::path`.
  // NOLINTNEXTLINE(readability-identifier-naming)
  [[nodiscard]] auto path() const -> const std::filesystem::path&;

 private:
  std::filesystem::path path_;
};

}  // namespace util::file_system::testing
