// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/flat_map/flat_map.h"

#include <array>
#include <cstddef>
#include <string_view>
#include <utility>

#include "gtest/gtest.h"
#include "util/numeric.h"

namespace {

TEST(FlatMap, At) {  // NOLINT
  using FlatMap = util::flat_map::FlatMap<std::string_view, int32, 3>;
  constexpr FlatMap map{{"one", 1}, {"two", 2}, {"three", 3}};
  for (const auto& [key, value] : map) {
    const auto retrieved_value = map.At(key);
    ASSERT_TRUE(retrieved_value.has_value());
    ASSERT_EQ(retrieved_value.value(), value);
  }
  const auto retrieved_value = map.At("zero");
  ASSERT_FALSE(retrieved_value.has_value());
}

}  // namespace
