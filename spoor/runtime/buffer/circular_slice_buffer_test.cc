#include "spoor/runtime/buffer/circular_slice_buffer.h"

#include <algorithm>
#include <initializer_list>

#include "gmock/gmock.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "util/numeric.h"

namespace {

using SlicePool = spoor::runtime::buffer::BufferSlicePool<uint64>;
using CircularBuffer = spoor::runtime::buffer::CircularBuffer<int64>;
using ValueType = CircularBuffer::ValueType;
using SizeType = CircularBuffer::SizeType;
using CircularSliceBuffer =
    spoor::runtime::buffer::CircularSliceBuffer<ValueType>;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<ValueType>;
using OwnedSlicePtr = util::memory::OwnedPtr<CircularBuffer>;

const SizeType kCapacity{10};

auto operator==(const std::vector<gsl::span<ValueType>>& result_chunks,
                const std::vector<std::vector<ValueType>>& expected_chunks)
    -> bool {
  if (std::size(result_chunks) != std::size(expected_chunks)) return false;
  for (SizeType index{0}; index < std::size(expected_chunks); ++index) {
    const auto result_chunk = result_chunks.at(index);
    const auto expected_chunk = expected_chunks.at(index);
    const auto equal =
        std::equal(std::cbegin(result_chunk), std::cend(result_chunk),
                   std::cbegin(expected_chunk));
    if (!equal) return false;
  }
  return true;
}

TEST(CircularSliceBuffer, ContiguousMemoryChunks) {  // NOLINT
  const typename Pool::Options options{.max_slice_capacity = 2,
                                       .capacity = kCapacity};
  Pool pool{options};
  const typename CircularSliceBuffer::Options circular_slice_buffer_options{
      .buffer_slice_pool = &pool, .capacity = kCapacity};
  CircularSliceBuffer circular_slice_buffer{circular_slice_buffer_options};

  ASSERT_TRUE(circular_slice_buffer.Empty());
  ASSERT_TRUE(circular_slice_buffer.ContiguousMemoryChunks().empty());
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(), {}));
  circular_slice_buffer.Push(0);
  ASSERT_TRUE(operator==
              (circular_slice_buffer.ContiguousMemoryChunks(), {{0}}));
  circular_slice_buffer.Push(1);
  ASSERT_TRUE(operator==
              (circular_slice_buffer.ContiguousMemoryChunks(), {{0, 1}}));
  circular_slice_buffer.Push(2);
  ASSERT_TRUE(operator==
              (circular_slice_buffer.ContiguousMemoryChunks(), {{0, 1}, {2}}));
  for (SizeType i{3}; i < kCapacity; ++i) {
    circular_slice_buffer.Push(i);
  }
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(),
                         {{0, 1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}}));
  circular_slice_buffer.Push(10);
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(),
                         {{1}, {2, 3}, {4, 5}, {6, 7}, {8, 9}, {10}}));
  circular_slice_buffer.Push(11);
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(),
                         {{2, 3}, {4, 5}, {6, 7}, {8, 9}, {10, 11}}));
  for (SizeType i{12}; i < 2 * kCapacity; ++i) {
    circular_slice_buffer.Push(i);
  }
  ASSERT_TRUE(operator==(circular_slice_buffer.ContiguousMemoryChunks(),
                         {{10, 11}, {12, 13}, {14, 15}, {16, 17}, {18, 19}}));
}

TEST(CircularSliceBuffer, PushWrapsIfBorrowFails) {  // NOLINT
  const typename Pool::Options options{.max_slice_capacity = 1,
                                       .capacity = kCapacity};
  Pool pool{options};
  const typename CircularSliceBuffer::Options circular_slice_buffer_options{
      .buffer_slice_pool = &pool,
      .capacity = kCapacity,
  };
  CircularSliceBuffer circular_slice_buffer_a{circular_slice_buffer_options};
  ASSERT_TRUE(circular_slice_buffer_a.Empty());
  for (SizeType i{0}; i < kCapacity; ++i) {
    circular_slice_buffer_a.Push(i);
  }
  ASSERT_TRUE(operator==(circular_slice_buffer_a.ContiguousMemoryChunks(),
                         {{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}}));

  CircularSliceBuffer circular_slice_buffer_b{circular_slice_buffer_options};
  ASSERT_TRUE(circular_slice_buffer_b.Empty());
  for (SizeType i{0}; i < kCapacity; ++i) {
    circular_slice_buffer_a.Push(i);
    ASSERT_TRUE(circular_slice_buffer_b.Empty());
  }

  circular_slice_buffer_a.Clear();
  ASSERT_TRUE(circular_slice_buffer_a.Empty());

  for (SizeType i{0}; i < kCapacity / 2; ++i) {
    circular_slice_buffer_a.Push(i);
  }
  ASSERT_TRUE(operator==(circular_slice_buffer_a.ContiguousMemoryChunks(),
                         {{0}, {1}, {2}, {3}, {4}}));
  for (SizeType i{0}; i < kCapacity; ++i) {
    circular_slice_buffer_b.Push(i);
  }
  ASSERT_TRUE(operator==(circular_slice_buffer_b.ContiguousMemoryChunks(),
                         {{5}, {6}, {7}, {8}, {9}}));
}

}  // namespace
