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
  constexpr int64 x{42};
  constexpr Optional optional{x};
  ASSERT_TRUE(optional.HasValue());
  ASSERT_EQ(optional.Value(), x);
}

TEST(Optional, RvalueConstructor) {  // NOLINT
  constexpr Optional optional{42};
  ASSERT_TRUE(optional.HasValue());
  ASSERT_EQ(optional.Value(), 42);
}

TEST(Optional, StdOptionalLvalueConstructor) {  // NOLINT
  {
    constexpr std::optional<int64> std_optional{42};
    constexpr Optional optional{std_optional};
    ASSERT_TRUE(optional.HasValue());
    ASSERT_EQ(optional.Value(), std_optional);
  }
  {
    constexpr std::optional<int64> std_optional{};
    constexpr Optional optional{std_optional};
    ASSERT_FALSE(optional.HasValue());
  }
}

TEST(Optional, StdOptionalRvalueConstructor) {  // NOLINT
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
  constexpr int64 x{42};
  constexpr Optional optional{x};
  ASSERT_EQ(optional.Value(), x);
}

TEST(Optional, ValueLvalue) {  // NOLINT
  constexpr int64 x{42};
  Optional optional{x};
  ASSERT_EQ(optional.Value(), x);
  optional.Value() += 1;
  ASSERT_EQ(optional.Value(), x + 1);
}

TEST(Optional, ValueConstRvalue) {  // NOLINT
  constexpr int64 x{42};
  constexpr Optional optional{x};
  // NOLINTNEXTLINE(performance-move-const-arg)
  ASSERT_EQ(std::move(optional).Value(), x);
}

TEST(Optional, ValueRvalue) {  // NOLINT
  constexpr int64 x{42};
  Optional optional{x};
  // NOLINTNEXTLINE(performance-move-const-arg)
  ASSERT_EQ(std::move(optional).Value(), x);
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
    constexpr int64 x{42};
    constexpr Optional optional{x};
    const auto std_optional = optional.StdOptional();
    ASSERT_EQ(std_optional, std::optional<int64>{x});
  }
}

}  // namespace
