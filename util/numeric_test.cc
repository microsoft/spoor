#include "util/numeric.h"

#include <gtest/gtest.h>

namespace {

TEST(Numeric, Int8) {  // NOLINT
  ASSERT_TRUE(std::is_integral_v<int8>);
  ASSERT_TRUE(std::is_signed_v<int8>);
  ASSERT_EQ(sizeof(int8), 1);
}

TEST(Numeric, Uint8) {  // NOLINT
  ASSERT_TRUE(std::is_integral_v<uint8>);
  ASSERT_FALSE(std::is_signed_v<uint8>);
  ASSERT_EQ(sizeof(uint8), 1);
}

TEST(Numeric, Int16) {  // NOLINT
  ASSERT_TRUE(std::is_integral_v<int16>);
  ASSERT_TRUE(std::is_signed_v<int16>);
  ASSERT_EQ(sizeof(int16), 2);
}

TEST(Numeric, Uint16) {  // NOLINT
  ASSERT_TRUE(std::is_integral_v<uint16>);
  ASSERT_FALSE(std::is_signed_v<uint16>);
  ASSERT_EQ(sizeof(uint16), 2);
}

TEST(Numeric, Int32) {  // NOLINT
  ASSERT_TRUE(std::is_integral_v<int32>);
  ASSERT_TRUE(std::is_signed_v<int32>);
  ASSERT_EQ(sizeof(int32), 4);
}

TEST(Numeric, Uint32) {  // NOLINT
  ASSERT_TRUE(std::is_integral_v<uint32>);
  ASSERT_FALSE(std::is_signed_v<uint32>);
  ASSERT_EQ(sizeof(uint32), 4);
}

TEST(Numeric, Int64) {  // NOLINT
  ASSERT_TRUE(std::is_integral_v<int64>);
  ASSERT_TRUE(std::is_signed_v<int64>);
  ASSERT_EQ(sizeof(int64), 8);
}

TEST(Numeric, Uint64) {  // NOLINT
  ASSERT_TRUE(std::is_integral_v<uint64>);
  ASSERT_FALSE(std::is_signed_v<uint64>);
  ASSERT_EQ(sizeof(uint64), 8);
}

}  // namespace