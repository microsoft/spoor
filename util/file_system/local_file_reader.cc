// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/file_system/local_file_reader.h"

#include <algorithm>
#include <filesystem>
#include <ios>

#include "gsl/gsl"

namespace util::file_system {

auto LocalFileReader::Open(const std::filesystem::path& path) -> void {
  ifstream_.open(path.c_str());
}

auto LocalFileReader::Open(const std::filesystem::path& path,
                           std::ios_base::openmode mode) -> void {
  ifstream_.open(path.c_str(), mode);
}

auto LocalFileReader::IsOpen() const -> bool { return ifstream_.is_open(); }

auto LocalFileReader::Read() -> std::string {
  std::string buffer{};
  ifstream_.seekg(0, std::ios::end);
  buffer.reserve(ifstream_.tellg());
  ifstream_.seekg(0, std::ios::beg);
  buffer.assign(std::istreambuf_iterator<char>{ifstream_},
                std::istreambuf_iterator<char>{});
  return buffer;
}

auto LocalFileReader::Read(const std::streamsize max_bytes) -> std::string {
  std::string buffer{};
  if (ifstream_.good()) {
    buffer.reserve(max_bytes);
    ifstream_.seekg(0, std::ios::beg);
    std::copy_n(std::istreambuf_iterator(ifstream_), max_bytes,
                std::back_inserter(buffer));
  }
  return buffer;
}

auto LocalFileReader::Istream() -> std::istream& { return ifstream_; }

auto LocalFileReader::Close() -> void { ifstream_.close(); }

}  // namespace util::file_system
