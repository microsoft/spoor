#include "spoor/runtime/buffer/circular_slice_buffer.h"

#include "gtest/gtest.h"
#include "util/numeric.h"

namespace {

using SlicePool = spoor::runtime::buffer::BufferSlicePool<uint64>;

}  // namespace
