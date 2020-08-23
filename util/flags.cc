#include "util/flags.h"

#include <fstream>

namespace util::flags {

auto ValidateInputFilePath(const char* /* flag_name */, const std::string& path)
    -> bool {
  // Prefer this technique over `std::filesystem::exists` for portability.
  const std::ifstream file{path};
  const auto exists = !file.fail();
  return exists;
}

auto ValidateOutputFilePath(const char* /* flag_name */,
                            const std::string& path) -> bool {
  return !path.empty();
}

}  // namespace util::flags
