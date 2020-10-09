#include "spoor/lib.h"

#include "gtest/gtest.h"

namespace {

using spoor::lib::Add;

TEST(Add, SumsTwoNumbers) {  // NOLINT
  ASSERT_EQ(Add(1, 2), 3);
}

}  // namespace
