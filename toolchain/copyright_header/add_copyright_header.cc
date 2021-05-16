// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "util/flat_map/flat_map.h"
#include "util/numeric.h"
#include "util/result.h"

namespace {

using Result = util::result::Result<util::result::None, util::result::None>;

constexpr std::string_view kCopyrightHeaderFile{
    "toolchain/copyright_header/copyright_header.txt"};
constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 19>
    kFileExtensionComment{
        {"BUILD", "#"},
        {"WORKSPACE", "#"},
        {"bazelrc", "#"},
        {"bzl", "#"},
        {"c", "//"},
        {"cc", "//"},
        {".clang-format", "#"},
        {".clang-tidy", "#"},
        {".gitignore", "#"},
        {"h", "//"},
        {"ll", ";"},
        {"m", "//"},
        {"mm", "//"},
        {"proto", "//"},
        {"py", "#"},
        {"sh", "#"},
        {"requirements-dev.txt", "#"},
        {"requirements.txt", "#"},
        {"yapf", "#"},
        {"yml", "#"},
    };
constexpr std::array<std::string_view, 6> kPathBlocklist{
    ".git", "bazel-bin", "bazel-out", "bazel-spoor", "bazel-testlogs", "dist"};

auto AddCopyrightHeaderToFile(const std::filesystem::path& path,
                              const std::string& copyright_header_text)
    -> Result {
  const auto extension = [&path] {
    const auto& file_name = path.filename();
    if (file_name.has_extension()) {
      return file_name.extension().string().substr(1);
    }
    return file_name.string();
  };
  auto comment =
      kFileExtensionComment.FirstValueForKey(path.filename().string());
  if (!comment.has_value()) {
    comment = kFileExtensionComment.FirstValueForKey(extension());
  }
  if (!comment.has_value()) return Result::Ok({});

  const auto copyright_header = [comment = comment, &copyright_header_text] {
    std::istringstream stream{copyright_header_text};
    std::ostringstream copyright_header{};
    std::string line{};
    while (std::getline(stream, line)) {
      copyright_header << *comment;
      if (!comment->empty()) copyright_header << ' ';
      copyright_header << line << '\n';
    }
    return copyright_header.str();
  }();

  std::string file_content{};
  std::string shebang_line{};
  {
    std::ifstream file_stream{path};
    if (!file_stream.is_open()) return Result::Err({});
    std::ostringstream file_content_stream{};
    std::string line{};
    constexpr std::string_view shebang{"#!"};
    int32 line_number{0};
    while (std::getline(file_stream, line)) {
      const auto starts_with_shebang =
          line.compare(0, shebang.size(), shebang) == 0;
      if (line_number == 0 && starts_with_shebang) {
        shebang_line = line;
        shebang_line.append("\n");
      } else if (!(line_number == 1 && line.empty() && !shebang_line.empty())) {
        file_content_stream << line << '\n';
      }
      file_content = file_content_stream.str();
      ++line_number;
    }
  }
  const auto contains_copyright_header =
      file_content.compare(0, copyright_header.size(), copyright_header) == 0;
  if (contains_copyright_header) return Result::Ok({});
  {
    std::ofstream file_stream{path, std::ios::out | std::ios::trunc};
    if (!file_stream.is_open()) return Result::Err({});
    file_stream << shebang_line;
    file_stream << copyright_header;
    if (!file_content.empty()) file_stream << '\n';
    file_stream << file_content;
  }

  return Result::Ok({});
}

}  // namespace

auto main(const int argc, const char** argv) -> int {
  if (argc != 2) {
    std::cerr << "Usage: " << std::next(argv, 0) << " [PATH]\n";
    return EXIT_FAILURE;
  }

  std::ifstream copyright_header_file{kCopyrightHeaderFile};
  if (!copyright_header_file.is_open()) {
    std::cerr << "Failed to open the copyright header file "
              << kCopyrightHeaderFile << ".\n";
    return EXIT_FAILURE;
  }
  const auto copyright_header_text = [&copyright_header_file] {
    std::stringstream buffer{};
    buffer << copyright_header_file.rdbuf();
    return buffer.str();
  }();

  bool ok{true};
  const std::string_view path{*std::next(argv, 1)};
  for (auto iterator = std::filesystem::recursive_directory_iterator(path);
       iterator != std::filesystem::recursive_directory_iterator();
       ++iterator) {
    const auto entry = *iterator;
    if (entry.is_directory() &&
        std::find(std::cbegin(kPathBlocklist), std::cend(kPathBlocklist),
                  entry.path().filename()) != std::cend(kPathBlocklist)) {
      iterator.disable_recursion_pending();
      continue;
    }
    if (entry.is_regular_file()) {
      const auto result =
          AddCopyrightHeaderToFile(entry.path(), copyright_header_text);
      ok &= result.IsOk();
      if (result.IsErr()) {
        std::cerr << "Failed to add a copyright header to the file "
                  << entry.path() << ".\n";
      }
    }
  }
  return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
