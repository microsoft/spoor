// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <fstream>
#include <ios>

#include "util/file_system/file_writer.h"

namespace util::file_system {

class LocalFileWriter final : public FileWriter {
 public:
  auto Open(const std::filesystem::path& path, std::ios_base::openmode mode)
      -> void override;
  [[nodiscard]] auto IsOpen() const -> bool override;
  auto Write(gsl::span<const char> data) -> void override;
  auto Close() -> void override;

 private:
  std::ofstream ofstream_;
};

}  // namespace util::file_system
