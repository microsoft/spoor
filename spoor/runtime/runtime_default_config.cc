// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <optional>
#include <string>

namespace spoor::runtime {

__attribute__((weak)) auto ConfigFilePath() -> std::optional<std::string> {
  return {};
}

}  // namespace spoor::runtime
