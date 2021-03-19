// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <ios>

#include "gsl/gsl"

namespace util::file_system {

class FileWriter {
 public:
  FileWriter() = default;
  FileWriter(const FileWriter&) = default;
  FileWriter(FileWriter&&) noexcept = default;
  auto operator=(const FileWriter&) -> FileWriter& = default;
  auto operator=(FileWriter&&) noexcept -> FileWriter& = default;
  virtual ~FileWriter() = default;

  virtual auto Open(const std::filesystem::path& path,
                    std::ios_base::openmode mode) -> void = 0;
  [[nodiscard]] virtual auto IsOpen() const -> bool = 0;
  virtual auto Write(gsl::span<const char> data) -> void = 0;
  virtual auto Close() -> void = 0;
};

}  // namespace util::file_system
