#include <gtest/gtest.h>

#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "util/numeric.h"

namespace {

using spoor::runtime::buffer::BufferSlicePool;

TEST(BufferSlicePool, Asdf) {  // NOLINT
  // BufferSlicePool<int64, 1> pool{1};
  // auto* borrowed = pool.BorrowSlice();
  // pool.ReturnSlice(borrowed);
}

}  // namespace
