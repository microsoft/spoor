// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <fstream>
#include <ios>
#include <ostream>

#include "util/file_system/file_writer.h"

namespace util::file_system {

class LocalFileWriter final : public FileWriter {
 public:
  LocalFileWriter() = default;
  LocalFileWriter(const LocalFileWriter&) = delete;
  LocalFileWriter(LocalFileWriter&&) noexcept = default;
  auto operator=(const LocalFileWriter&) -> LocalFileWriter& = delete;
  auto operator=(LocalFileWriter&&) noexcept -> LocalFileWriter& = default;
  ~LocalFileWriter() override = default;

  auto Open(const std::filesystem::path& path) -> void override;
  auto Open(const std::filesystem::path& path, std::ios_base::openmode mode)
      -> void override;
  [[nodiscard]] auto IsOpen() const -> bool override;
  auto Write(gsl::span<const char> data) -> void override;
  auto Ostream() -> std::ostream& override;
  auto Close() -> void override;

 private:
  std::ofstream ofstream_;
};

}  // namespace util::file_system
