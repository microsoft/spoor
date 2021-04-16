// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/flags/optional.h"

#include <string>

#include "absl/strings/string_view.h"

namespace util::flags {

template <>
constexpr Optional<std::string>::Optional(const std::string& item)
    : item_{[&item]() -> std::optional<std::string> {
        if (item.empty()) return {};
        return item;
      }()} {}

template <>
constexpr Optional<std::string>::Optional(std::string&& item)
    : item_{[&item]() -> std::optional<std::string> {
        if (item.empty()) return {};
        return std::move(item);
      }()} {}

template <>
auto AbslParseFlag(const absl::string_view user_value,
                   Optional<std::string>* optional, std::string*
                   /*error*/) -> bool {
  if (user_value.empty()) {
    *optional = {};
  } else {
    *optional = std::string{user_value};
  }
  return true;
}

}  // namespace util::flags
