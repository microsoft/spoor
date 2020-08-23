#ifndef SPOOR_UTIL_FLAGS_H_
#define SPOOR_UTIL_FLAGS_H_

#include <string>

namespace util::flags {

auto ValidateInputFilePath(const char* /* flag_name */, const std::string& path)
    -> bool;
auto ValidateOutputFilePath(const char* /* flag_name */,
                            const std::string& path) -> bool;

}  // namespace util::flags

#endif
