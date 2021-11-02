// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/env/env.h"

#include <optional>
#include <string>
#include <string_view>

#include "absl/strings/ascii.h"
#include "absl/strings/numbers.h"

namespace util::env {

auto GetEnv(std::string_view key, const bool empty_string_is_nullopt,
            const StdGetEnv& get_env)
    -> std::optional<util::result::Result<std::string, util::result::None>> {
  using Result = util::result::Result<std::string, util::result::None>;
  const auto* user_value = get_env(key.data());
  if (user_value == nullptr) return {};
  const std::string user_value_string{user_value};
  if (empty_string_is_nullopt && user_value_string.empty()) return {};
  return Result::Ok(user_value_string);
}

}  // namespace util::env
