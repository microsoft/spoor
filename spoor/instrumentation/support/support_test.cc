// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/support/support.h"

#include <sstream>
#include <unordered_set>

#include "gtest/gtest.h"

namespace {

using spoor::instrumentation::support::ReadLinesToSet;

TEST(Support, ReadLinesToSet) {  // NOLINT
  std::stringstream stream{"bar\nbaz\nfoo\nbar"};
  ASSERT_FALSE(stream.eof());
  std::unordered_set<std::string> expected_unique_lines{"foo", "bar", "baz"};
  const auto unique_lines = ReadLinesToSet(&stream);
  ASSERT_EQ(unique_lines, expected_unique_lines);
  ASSERT_TRUE(stream.eof());

  std::stringstream empty_stream{};
  const auto empty_lines = ReadLinesToSet(&stream);
  ASSERT_TRUE(empty_lines.empty());
}

}  // namespace
