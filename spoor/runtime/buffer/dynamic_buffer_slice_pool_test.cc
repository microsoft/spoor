#include "spoor/runtime/buffer/dynamic_buffer_slice_pool.h"

#include <future>
#include <limits>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "spoor/runtime/buffer/buffer_slice.h"
#include "util/memory/owned_ptr.h"
#include "util/numeric.h"
#include "util/result.h"

namespace {

using Slice = spoor::runtime::buffer::BufferSlice<int64>;
using ValueType = Slice::ValueType;
using SizeType = Slice::SizeType;
using Pool = spoor::runtime::buffer::DynamicBufferSlicePool<ValueType>;
using Options = typename Pool::Options;
using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;

TEST(DynamicBufferSlicePool, BorrowsPreferredSliceCapacity) {  // NOLINT
  const std::vector<SizeType> capacities{1, 10, 100, 1'000};
  const Options options{
      .max_slice_capacity = std::numeric_limits<ValueType>::max(),
      .capacity = std::numeric_limits<ValueType>::max(),
      .borrow_cas_attempts = 1};
  Pool pool{options};
  std::vector<OwnedSlicePtr> retained_slices{};
  retained_slices.reserve(capacities.size());
  for (const auto size : capacities) {
    ASSERT_LT(size, options.max_slice_capacity);
    ASSERT_LT(size, options.capacity);
    auto result = pool.Borrow(size);
    ASSERT_TRUE(result.IsOk());
    auto slice = std::move(result.Ok());
    ASSERT_EQ(slice->Capacity(), size);
    retained_slices.push_back(std::move(slice));
  }
}

TEST(DynamicBufferSlicePool,  // NOLINT
     BorrowsMaximumAllowedDynamicSliceCapacity) {
  for (const SizeType capacity : {1, 10, 100, 1'000}) {
    const Options options{.max_slice_capacity = capacity,
                          .capacity = std::numeric_limits<ValueType>::max(),
                          .borrow_cas_attempts = 1};
    Pool pool{options};
    auto result = pool.Borrow(capacity + 1);
    ASSERT_TRUE(result.IsOk());
    auto slice = std::move(result.Ok());
    ASSERT_EQ(slice->Capacity(), capacity);
  }
}

TEST(DynamicBufferSlicePool, BorrowsRemainingSlicesCapacity) {  // NOLINT
  for (const SizeType capacity : {1, 10, 100, 1'000}) {
    for (const SizeType extra_capacity : {1, 10, 100, 1'000}) {
      if (capacity < extra_capacity) continue;
      const Options options{
          .max_slice_capacity = std::numeric_limits<ValueType>::max(),
          .capacity = capacity + extra_capacity,
          .borrow_cas_attempts = 1};
      Pool pool{options};
      auto full_capacity_result = pool.Borrow(capacity);
      ASSERT_TRUE(full_capacity_result.IsOk());
      auto full_capacity_slice = std::move(full_capacity_result.Ok());
      ASSERT_EQ(full_capacity_slice->Capacity(), capacity);
      auto remaining_capacity_result = pool.Borrow(capacity);
      ASSERT_TRUE(remaining_capacity_result.IsOk());
      auto remaining_capacity_slice = std::move(remaining_capacity_result.Ok());
      ASSERT_EQ(remaining_capacity_slice->Capacity(), extra_capacity);
    }
  }
}

TEST(DynamicBufferSlicePool, BorrowReturnsErrWhenNoSlicesAvailable) {  // NOLINT
  for (const SizeType capacity : {1, 10, 100, 1'000}) {
    const Options options{.max_slice_capacity = capacity,
                          .capacity = 0,
                          .borrow_cas_attempts = 1};
    Pool pool{options};
    auto result = pool.Borrow(capacity);
    ASSERT_TRUE(result.IsErr());
    ASSERT_EQ(result.Err(), Pool::BorrowError::kNoSlicesAvailable);
  }
  for (const SizeType capacity : {1, 10, 100, 1'000}) {
    const Options options{.max_slice_capacity = capacity,
                          .capacity = capacity,
                          .borrow_cas_attempts = 1};
    Pool pool{options};
    auto ok_result = pool.Borrow(capacity);
    ASSERT_TRUE(ok_result.IsOk());
    auto slice = std::move(ok_result.Ok());
    ASSERT_EQ(slice->Capacity(), capacity);
    auto err_result = pool.Borrow(1);
    ASSERT_TRUE(err_result.IsErr());
    ASSERT_EQ(err_result.Err(), Pool::BorrowError::kNoSlicesAvailable);
  }
}

}  // namespace
