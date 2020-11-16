#include "util/env/env.h"

#include <algorithm>
#include <cctype>
#include <string>

namespace util::env {

auto GetEnvOrDefault(const char* key, std::string default_value,
                     const GetEnv& get_env) -> std::string {
  const auto* user_value = get_env(key);
  if (user_value == nullptr) return default_value;
  return std::string{user_value};
}

auto GetEnvOrDefault(const char* key, const bool default_value,
                     const GetEnv& get_env) -> bool {
  const auto* user_value = get_env(key);
  if (user_value == nullptr) return default_value;
  std::string value{user_value};
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  if (value == "0" || value == "no" || value == "false") return false;
  if (value == "1" || value == "yes" || value == "true") return true;
  return default_value;
}

}  // namespace util::env
