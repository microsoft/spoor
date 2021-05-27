// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <ios>
#include <istream>
#include <string>

#include "gsl/gsl"

namespace util::file_system {

class FileReader {
 public:
  FileReader() = default;
  FileReader(const FileReader&) = default;
  FileReader(FileReader&&) noexcept = default;
  auto operator=(const FileReader&) -> FileReader& = default;
  auto operator=(FileReader&&) noexcept -> FileReader& = default;
  virtual ~FileReader() = default;

  virtual auto Open(const std::filesystem::path& path,
                    std::ios_base::openmode mode) -> void = 0;
  [[nodiscard]] virtual auto IsOpen() const -> bool = 0;
  virtual auto Read() -> std::string = 0;
  virtual auto Read(std::streamsize max_bytes) -> std::string = 0;
  virtual auto Istream() -> std::istream& = 0;
  virtual auto Close() -> void = 0;
};

}  // namespace util::file_system
