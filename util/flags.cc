#include "util/flags.h"

#include <string>

#include "absl/strings/string_view.h"

namespace util::flags {

auto AbslUnparseFlag(const InputFilePath& input_path) -> std::string {
  return absl::UnparseFlag(input_path.input_path);
}

auto AbslParseFlag(absl::string_view text, OutputPath* output_path,
                   std::string* error) -> bool {
  auto& path = output_path->output_path;
  const auto success = absl::ParseFlag(text, &path, error);
  if (!success) return false;
  if (!path.empty()) return true;
  *error = "The output path must not be empty.";
  return false;
}

auto AbslUnparseFlag(const OutputPath& output_path) -> std::string {
  return absl::UnparseFlag(output_path.output_path);
}

}  // namespace util::flags
