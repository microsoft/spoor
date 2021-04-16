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

TEST(FlatMap, KeyValueLookup) {  // NOLINT
  using FlatMap = util::flat_map::FlatMap<std::string_view, int32, 3>;
  constexpr FlatMap map{{"one", 1}, {"two", 2}, {"three", 3}};
  for (const auto& [key, value] : map) {
    const auto retrieved_value = map.FirstValueForKey(key);
    ASSERT_TRUE(retrieved_value.has_value());
    ASSERT_EQ(retrieved_value.value(), value);
    const auto retrieved_key = map.FirstKeyForValue(value);
    ASSERT_TRUE(retrieved_key.has_value());
    ASSERT_EQ(retrieved_key.value(), key);
  }
  const auto retrieved_value = map.FirstValueForKey("zero");
  ASSERT_FALSE(retrieved_value.has_value());
  const auto retrieved_key = map.FirstKeyForValue(0);
  ASSERT_FALSE(retrieved_key.has_value());
}

TEST(FlatMap, GetKeysValues) {  // NOLINT
  using FlatMap = util::flat_map::FlatMap<std::string_view, int32, 3>;
  constexpr FlatMap map{{"one", 1}, {"two", 2}, {"three", 3}};
  constexpr std::array<std::string_view, 3> expected_keys{
      {"one", "two", "three"}};
  constexpr std::array<int32, 3> expected_values{{1, 2, 3}};
  ASSERT_EQ(map.Keys(), expected_keys);
  ASSERT_EQ(map.Values(), expected_values);
}

TEST(FlatMap, Size) {  // NOLINT
  constexpr util::flat_map::FlatMap<std::string_view, int32, 0> zero{};
  constexpr util::flat_map::FlatMap<std::string_view, int32, 1> one{{"one", 1}};
  constexpr util::flat_map::FlatMap<std::string_view, int32, 2> two{{"one", 1},
                                                                    {"two", 2}};
  ASSERT_EQ(zero.size(), 0);
  ASSERT_EQ(one.size(), 1);
  ASSERT_EQ(two.size(), 2);
}

}  // namespace
