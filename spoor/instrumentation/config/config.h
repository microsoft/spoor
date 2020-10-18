#pragma once

#include <cstdlib>
#include <string_view>

#include "util/env.h"

namespace spoor::instrumentation::config {

// constexpr std::string_view kKey{
//     "SPOOR_INSTRUMENTATION_MIN_INSTRUMENTATION_INSTRUCTION_THRESHOLD"};
// constexpr std::string_view kKey{"SPOOR_INSTRUMENTATION_FUNCTION_BLOCKLIST"};
// constexpr std::string_view kKey{
//     "SPOOR_INSTRUMENTATION_FUNCTION_BLOCKLIST_FILE"};
// constexpr std::string_view kKey{"SPOOR_INSTRUMENTATION_FUNCTION_ALLOWLIST"};
// constexpr std::string_view kKey{
//     "SPOOR_INSTRUMENTATION_FUNCTION_ALLOWLIST_FILE"};
// constexpr std::string_view kKey{"SPOOR_INSTRUMENTATION_BINARY_ID"};
// constexpr std::string_view kKey{
//     "SPOOR_INSTRUMENTATION_FUNCTION_DEMANGLE_LANGUAGES"};
// constexpr std::string_view kKey{"SPOOR_INSTRUMENTATION_FUNCTION_MAP_ID"};
// constexpr std::string_view kKey{
//     "SPOOR_INSTRUMENTATION_FUNCTION_ID_MAP_FILE_PATH"};
// constexpr std::string_view kKey{
//     "SPOOR_INSTRUMENTATION_RUNTIME_INITIALIZATION_IN_MAIN"};

struct Config {
  static auto FromEnv(const util::env::GetEnv& get_env = [](const char* key) {
    return std::getenv(key);
  }) -> Config;
};

}  // namespace spoor::instrumentation::config
