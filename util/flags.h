#ifndef SPOOR_UTIL_FLAGS_H_
#define SPOOR_UTIL_FLAGS_H_

#include <fstream>
#include <string>
#include <iostream>  // TODO

#include "absl/flags/flag.h"
#include "absl/flags/marshalling.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"

namespace util::flags {

struct InputFilePath {
  explicit InputFilePath(std::string path = {}) : input_path(std::move(path)) {}

  std::string input_path;
};

struct OutputPath {
  std::string output_path;
};

template <class Ifstream = std::ifstream>
auto AbslParseFlag(absl::string_view text, InputFilePath* input_path,
                   std::string* error) -> bool;
auto AbslUnparseFlag(const InputFilePath& input_file_path) -> std::string;

auto AbslParseFlag(absl::string_view text, OutputPath* output_path,
                   std::string* error) -> bool;
auto AbslUnparseFlag(const OutputPath& output_path) -> std::string;

template <class Ifstream>  // NOLINT(readability-identifier-naming)
auto AbslParseFlag(absl::string_view text, InputFilePath* input_file_path,
                   std::string* error) -> bool {
std::cerr << "text = " << text << '\n';
  auto& path = input_file_path->input_path;
  const auto success = absl::ParseFlag(text, &path, error);
  if (!success) return false;
  if (text.empty()) return true;
  const Ifstream file{path};
  // Prefer this technique over `std::filesystem::exists` for platform
  // portability.
  const auto exists = !file.fail();
  if (exists) return true;
  *error = absl::StrCat("The input file `", path, "` does not exist.");
  return false;
}

}  // namespace util::flags

#endif
