// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/file_system/local_file_writer.h"

#include <filesystem>
#include <ios>

#include "gsl/gsl"

namespace util::file_system {

auto LocalFileWriter::Open(const std::filesystem::path& path) -> void {
  ofstream_.open(path.c_str());
}

auto LocalFileWriter::Open(const std::filesystem::path& path,
                           std::ios_base::openmode mode) -> void {
  ofstream_.open(path.c_str(), mode);
}

auto LocalFileWriter::IsOpen() const -> bool { return ofstream_.is_open(); }

auto LocalFileWriter::Write(const gsl::span<const char> data) -> void {
  ofstream_.write(data.data(),
                  gsl::narrow_cast<std::streamsize>(data.size_bytes()));
}

auto LocalFileWriter::Ostream() -> std::ostream& { return ofstream_; }

auto LocalFileWriter::Close() -> void { ofstream_.close(); }

}  // namespace util::file_system
