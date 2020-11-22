#include "util/file_system/directory_entry_mock.h"

#include <filesystem>
#include <utility>

namespace util::file_system::testing {

DirectoryEntryMock::DirectoryEntryMock(std::filesystem::path path)
    : path_{std::move(path)} {}

auto DirectoryEntryMock::path() const -> const std::filesystem::path& {
  return path_;
}

}  // namespace util::file_system::testing
