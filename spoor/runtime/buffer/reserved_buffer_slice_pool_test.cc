#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"

#include <vector>

#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_buffer.h"
#include "spoor/runtime/buffer/owned_buffer_slice.h"
#include "util/memory/owned_ptr.h"
#include "util/numeric.h"

namespace {

using Slice = spoor::runtime::buffer::CircularBuffer<int64>;
using OwnedBufferSlice = spoor::runtime::buffer::OwnedBufferSlice<int64>;
using ValueType = Slice::ValueType;
using SizeType = Slice::SizeType;
using Pool = spoor::runtime::buffer::ReservedBufferSlicePool<ValueType>;
using Options = typename Pool::Options;
using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
using PtrOwner = util::memory::PtrOwner<Slice>;

const SizeType kIgnore = 0;

TEST(ReservedBufferSlicePool, BorrowsMaxSliceCapacity) {  // NOLINT
  const SizeType slices_size{1'000};
  for (SizeType capacity : {1, 2, 5, 10}) {
    const Options options{.max_slice_capacity = capacity,
                          .capacity = slices_size * capacity};
    Pool pool{options};
    std::vector<OwnedSlicePtr> retained_slices{};
    retained_slices.reserve(capacity);
    for (SizeType i{0}; i < slices_size; ++i) {
      auto result = pool.Borrow(kIgnore);
      ASSERT_TRUE(result.IsOk());
      auto slice = std::move(result.Ok());
      ASSERT_EQ(slice->Capacity(), capacity);
      retained_slices.push_back(std::move(slice));
    }
  }
}

TEST(ReservedBufferSlicePool, BorrowsRemainingSliceCapacity) {  // NOLINT
  for (const SizeType capacity : {1, 10, 100, 1'000}) {
    for (const SizeType extra_capacity : {1, 10, 100, 1'000}) {
      if (capacity < extra_capacity) continue;
      const Options options{.max_slice_capacity = capacity,
                            .capacity = capacity + extra_capacity};
      Pool pool{options};
      ASSERT_EQ(pool.Capacity(), capacity + extra_capacity);
      ASSERT_EQ(pool.Size(), pool.Capacity());
      auto full_capacity_result = pool.Borrow(kIgnore);
      ASSERT_TRUE(full_capacity_result.IsOk());
      auto full_capacity_slice = std::move(full_capacity_result.Ok());
      ASSERT_EQ(full_capacity_slice->Capacity(), capacity);
      auto remaining_capacity_result = pool.Borrow(kIgnore);
      ASSERT_EQ(pool.Size(), 0);
      ASSERT_TRUE(remaining_capacity_result.IsOk());
      auto remaining_capacity_slice = std::move(remaining_capacity_result.Ok());
      ASSERT_EQ(remaining_capacity_slice->Capacity(), extra_capacity);
    }
  }
}

// NOLINTNEXTLINE
TEST(ReservedBufferSlicePool, BorrowReturnsErrWhenNoSlicesAvailable) {
  for (const SizeType capacity : {1, 10, 100, 1'000}) {
    const Options options{.max_slice_capacity = capacity, .capacity = 0};
    Pool pool{options};
    auto result = pool.Borrow(kIgnore);
    ASSERT_TRUE(result.IsErr());
    ASSERT_EQ(result.Err(), Pool::BorrowError::kNoSlicesAvailable);
  }
  for (const SizeType capacity : {1, 10, 100, 1'000}) {
    const Options options{.max_slice_capacity = capacity, .capacity = capacity};
    Pool pool{options};
    auto ok_result = pool.Borrow(kIgnore);
    ASSERT_TRUE(ok_result.IsOk());
    auto slice = std::move(ok_result.Ok());
    ASSERT_EQ(slice->Capacity(), capacity);
    auto err_result = pool.Borrow(kIgnore);
    ASSERT_TRUE(err_result.IsErr());
    ASSERT_EQ(err_result.Err(), Pool::BorrowError::kNoSlicesAvailable);
  }
}

}  // namespace
