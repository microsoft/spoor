// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

// This test will fail to compile if we do not implement one of the methods.

#include <optional>
#include <string>

#include "gtest/gtest.h"

namespace spoor::runtime {

auto ConfigFilePath() -> std::optional<std::string> {
  return "/path/to/spoor_runtime_config.toml";
}

}  // namespace spoor::runtime

namespace {

TEST(Runtime, ConfigFilePath) {  // NOLINT
  ASSERT_NE(spoor::runtime::ConfigFilePath(), std::nullopt);
}

}  // namespace
