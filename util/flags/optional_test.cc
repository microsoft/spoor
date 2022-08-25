// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "util/flags/optional.h"

#include <optional>

#include "gtest/gtest.h"
#include "util/numeric.h"

namespace {

using Optional = util::flags::Optional<int64>;

TEST(Optional, DefaultConstructor) {  // NOLINT
  constexpr Optional optional{};
  ASSERT_FALSE(optional.HasValue());
}

TEST(Optional, LvalueConstructor) {  // NOLINT
  constexpr int64 value{42};
  constexpr Optional optional{value};
  ASSERT_TRUE(optional.HasValue());
  ASSERT_EQ(optional.Value(), value);
}

TEST(Optional, RvalueConstructor) {  // NOLINT
  constexpr Optional optional{42};
  ASSERT_TRUE(optional.HasValue());
  ASSERT_EQ(optional.Value(), 42);
}

TEST(Optional, StdOptionalConstructor) {  // NOLINT
  {
    constexpr Optional optional{std::optional<int64>{42}};
    ASSERT_TRUE(optional.HasValue());
    ASSERT_EQ(optional.Value(), std::optional<int64>{42});
  }
  {
    constexpr Optional optional{std::optional<int64>{}};
    ASSERT_FALSE(optional.HasValue());
  }
}

TEST(Optional, HasValue) {  // NOLINT
  {
    constexpr Optional optional{42};
    ASSERT_TRUE(optional.HasValue());
  }
  {
    constexpr Optional optional{};
    ASSERT_FALSE(optional.HasValue());
  }
}

TEST(Optional, ValueConstLvalue) {  // NOLINT
  constexpr int64 value{42};
  constexpr Optional optional{value};
  ASSERT_EQ(optional.Value(), value);
}

TEST(Optional, ValueLvalue) {  // NOLINT
  constexpr int64 value{42};
  Optional optional{value};
  ASSERT_EQ(optional.Value(), value);
  optional.Value() += 1;
  ASSERT_EQ(optional.Value(), value + 1);
}

TEST(Optional, ValueConstRvalue) {  // NOLINT
  constexpr int64 value{42};
  constexpr Optional optional{value};
  // NOLINTNEXTLINE(performance-move-const-arg)
  ASSERT_EQ(std::move(optional).Value(), value);
}

TEST(Optional, ValueRvalue) {  // NOLINT
  constexpr int64 value{42};
  Optional optional{value};
  // NOLINTNEXTLINE(performance-move-const-arg)
  ASSERT_EQ(std::move(optional).Value(), value);
}

TEST(Optional, ValueOrLvalue) {  // NOLINT
  {
    constexpr Optional optional{};
    const auto value = optional.ValueOr(42);
    ASSERT_EQ(value, 42);
  }
  {
    constexpr Optional optional{123};
    const auto value = optional.ValueOr(42);
    ASSERT_EQ(value, 123);
  }
}

TEST(Optional, ValueOrRvalue) {  // NOLINT
  {
    constexpr Optional optional{};
    // NOLINTNEXTLINE(performance-move-const-arg)
    const auto value = std::move(optional).ValueOr(42);
    ASSERT_EQ(value, 42);
  }
  {
    constexpr Optional optional{123};
    // NOLINTNEXTLINE(performance-move-const-arg)
    const auto value = std::move(optional).ValueOr(42);
    ASSERT_EQ(value, 123);
  }
}

TEST(Optional, StdOptional) {  // NOLINT
  {
    constexpr Optional optional{};
    const auto std_optional = optional.StdOptional();
    ASSERT_EQ(std_optional, std::optional<int64>{});
  }
  {
    constexpr int64 value{42};
    constexpr Optional optional{value};
    const auto std_optional = optional.StdOptional();
    ASSERT_EQ(std_optional, std::optional<int64>{value});
  }
}

}  // namespace
