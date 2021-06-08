// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <fstream>
#include <ios>

#include "util/file_system/file_reader.h"

namespace util::file_system {

class LocalFileReader final : public FileReader {
 public:
  LocalFileReader() = default;
  LocalFileReader(const LocalFileReader&) = delete;
  LocalFileReader(LocalFileReader&&) noexcept = default;
  auto operator=(const LocalFileReader&) -> LocalFileReader& = delete;
  auto operator=(LocalFileReader&&) noexcept -> LocalFileReader& = default;
  ~LocalFileReader() override = default;

  auto Open(const std::filesystem::path& path) -> void override;
  auto Open(const std::filesystem::path& path, std::ios_base::openmode mode)
      -> void override;
  [[nodiscard]] auto IsOpen() const -> bool override;
  auto Read() -> std::string override;
  auto Read(std::streamsize max_bytes) -> std::string override;
  auto Istream() -> std::istream& override;
  auto Close() -> void override;

 private:
  std::ifstream ifstream_;
};

}  // namespace util::file_system
