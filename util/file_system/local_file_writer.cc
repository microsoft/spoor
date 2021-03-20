// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/file_system/local_file_writer.h"

#include <filesystem>
#include <ios>

namespace util::file_system {

auto LocalFileWriter::Open(const std::filesystem::path& path,
                           std::ios_base::openmode mode) -> void {
  ofstream_.open(path.c_str(), mode);
}

auto LocalFileWriter::IsOpen() const -> bool { return ofstream_.is_open(); }

auto LocalFileWriter::Write(const gsl::span<const char> data) -> void {
  ofstream_.write(data.data(), data.size_bytes());
}

auto LocalFileWriter::Close() -> void { ofstream_.close(); }

}  // namespace util::file_system
