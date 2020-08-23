#include "util/strings.h"

#include <gtest/gtest.h>

#include <array>
#include <set>
#include <string>
#include <vector>

namespace {

using util::strings::Join;

TEST(Join, JoinsLists) {  // NOLINT
  ASSERT_EQ(Join(std::array{1, 2, 3}, ", "), "1, 2, 3");
  ASSERT_EQ(Join(std::vector{1, 2, 3}, ", "), "1, 2, 3");
  ASSERT_EQ(Join(std::set{1, 2, 3}, ", "), "1, 2, 3");
  ASSERT_EQ(Join(std::vector{'1', '2', '3'}, ", "), "1, 2, 3");
  ASSERT_EQ(Join(std::vector{"1", "2", "3"}, ", "), "1, 2, 3");
  ASSERT_EQ(Join(std::string{"123"}, ", "), "1, 2, 3");
}

TEST(Join, Delimiter) {  // NOLINT
  ASSERT_EQ(Join(std::array{1, 2, 3}, ""), "123");
  ASSERT_EQ(Join(std::array{1, 2, 3}, "-"), "1-2-3");
  ASSERT_EQ(Join(std::array{1, 2, 3}, " -> "), "1 -> 2 -> 3");
}

TEST(Join, EmptyListProducesEmptyString) {  // NOLINT
  ASSERT_EQ(Join(std::vector<int>{}, ""), "");
  ASSERT_EQ(Join(std::set<std::string>{}, ""), "");
}

}  // namespace