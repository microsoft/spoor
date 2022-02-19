// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <optional>
#include <string>

#include "gtest/gtest.h"

namespace spoor::runtime {

extern auto ConfigFilePath() -> std::optional<std::string>;

}  // namespace spoor::runtime

namespace {

TEST(Runtime, ConfigFilePath) {  // NOLINT
  ASSERT_EQ(spoor::runtime::ConfigFilePath(), std::optional<std::string>{});
}

}  // namespace
