#include "spoor/runtime/buffer/unowned_buffer_slice.h"

#include "util/numeric.h"

#include "gtest/gtest.h"

namespace {

using BufferSlice = spoor::runtime::buffer::CircularBuffer<int64>;
using ValueType = BufferSlice::ValueType;
using UnownedBufferSlice =
    spoor::runtime::buffer::UnownedBufferSlice<ValueType>;
using SizeType = BufferSlice::SizeType;

TEST(UnownedBufferSlice, Movable) {  // NOLINT
  std::vector<int64> buffer(10);
  UnownedBufferSlice buffer_slice{{buffer.data(), buffer.size()}};
}

}  // namespace
