#include "util/flags.h"

#include <gtest/gtest.h>

namespace {

using util::flags::ValidateInputFilePath;
using util::flags::ValidateOutputFilePath;

TEST(ValidateInputFilePath, ExistentePathsAreValid) {  // NOLINT
  // Naughty test: This test actually hits the file system.
  // Dependency injection with gMock is not an option because the function
  // interface must be compatible with gflags.
  ASSERT_FALSE(ValidateInputFilePath("", ""));
  ASSERT_FALSE(ValidateInputFilePath("", "/does-not-exist"));
  ASSERT_FALSE(ValidateInputFilePath("", "does-not-exist"));
  ASSERT_TRUE(ValidateInputFilePath("", "/"));
}

TEST(ValidateOutputFilePath, NonEmptyPathsAreValid) {  // NOLINT
  ASSERT_FALSE(ValidateOutputFilePath("", ""));
  ASSERT_TRUE(ValidateOutputFilePath("", "/"));
  ASSERT_TRUE(ValidateOutputFilePath("", "/foo/bar/baz"));
  ASSERT_TRUE(ValidateOutputFilePath("", "foo"));
}

}  // namespace