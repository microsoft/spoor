#include "util/mock/ifstream.h"

#include "gtest/gtest.h"

namespace {

TEST(Ifstream, DoesNotFailIfExists) {  // NOLINT
  util::mock::Ifstream<true> ifstream{"path"};
  ASSERT_FALSE(ifstream.fail());
}

TEST(Ifstream, FailsIfNotExists) {  // NOLINT
  util::mock::Ifstream<false> ifstream{"path"};
  ASSERT_TRUE(ifstream.fail());
}

}  // namespace
