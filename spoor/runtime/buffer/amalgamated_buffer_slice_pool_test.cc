#include "spoor/runtime/buffer/amalgamated_buffer_slice_pool.h"

#include <unordered_set>
#include <vector>

#include "gtest/gtest.h"
#include "spoor/runtime/buffer/circular_buffer.h"
#include "spoor/runtime/buffer/dynamic_buffer_slice_pool.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "util/memory/owned_ptr.h"
#include "util/numeric.h"

namespace {

using Slice = spoor::runtime::buffer::CircularBuffer<int64>;
using ValueType = Slice::ValueType;
using SizeType = Slice::SizeType;
using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
using BorrowError =
    spoor::runtime::buffer::BufferSlicePool<ValueType>::BorrowError;
using DynamicPoolOptions =
    spoor::runtime::buffer::DynamicBufferSlicePool<ValueType>::Options;
using ReservedPoolOptions =
    spoor::runtime::buffer::ReservedBufferSlicePool<ValueType>::Options;
using AmalgamatedPool =
    spoor::runtime::buffer::AmalgamatedBufferSlicePool<ValueType>;

TEST(AmalgamatedBufferSlicePool, BorrowSliceFromReservedPool) {  // NOLINT
  const SizeType dynamic_pool_capacity{0};
  for (const SizeType reserved_pool_capacity : {0, 1, 2, 5}) {
    const auto expected_capacity =
        reserved_pool_capacity + dynamic_pool_capacity;
    const ReservedPoolOptions reserved_pool_options{
        .max_slice_capacity = 1, .capacity = reserved_pool_capacity};
    const DynamicPoolOptions dynamic_pool_options{
        .max_slice_capacity = 1,
        .capacity = dynamic_pool_capacity,
        .borrow_cas_attempts = 1};
    const AmalgamatedPool::Options options{
        .reserved_pool_options = reserved_pool_options,
        .dynamic_pool_options = dynamic_pool_options};
    AmalgamatedPool pool{options};
    ASSERT_EQ(pool.Capacity(), expected_capacity);
    ASSERT_EQ(pool.Size(), pool.Capacity());
    std::vector<OwnedSlicePtr> retained_slices{};
    retained_slices.reserve(reserved_pool_capacity);
    std::unordered_set<Slice*> unique_slice_ptrs{};
    unique_slice_ptrs.reserve(reserved_pool_capacity);
    for (SizeType i{0}; i < reserved_pool_capacity; ++i) {
      auto result = pool.Borrow(1);
      ASSERT_TRUE(result.IsOk());
      auto slice = std::move(result.Ok());
      ASSERT_NE(slice.Ptr(), nullptr);
      ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity - i - 1);
      ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity);
      ASSERT_EQ(pool.Size(), pool.DynamicPoolSize() + pool.ReservedPoolSize());
      ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
      ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
      ASSERT_EQ(pool.Capacity(), expected_capacity);
      unique_slice_ptrs.insert(slice.Ptr());
      retained_slices.push_back(std::move(slice));
    }
    ASSERT_EQ(unique_slice_ptrs.size(), reserved_pool_capacity);
    for (SizeType i{0}; i < 3; ++i) {
      auto result = pool.Borrow(1);
      ASSERT_TRUE(result.IsErr());
      ASSERT_EQ(result.Err(), BorrowError::kNoSlicesAvailable);
      ASSERT_EQ(pool.ReservedPoolSize(), 0);
      ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity);
      ASSERT_EQ(pool.Size(), 0);
      ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
      ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
      ASSERT_EQ(pool.Capacity(), expected_capacity);
    }
    for (SizeType i{0}; i < reserved_pool_capacity; ++i) {
      auto slice = std::move(retained_slices.back());
      retained_slices.pop_back();
      unique_slice_ptrs.erase(slice.Ptr());
      const auto result = pool.Return(std::move(slice));
      ASSERT_TRUE(result.IsOk());
      ASSERT_EQ(pool.ReservedPoolSize(), i + 1);
      ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity);
      ASSERT_EQ(pool.Size(), pool.DynamicPoolSize() + pool.ReservedPoolSize());
      ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
      ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
      ASSERT_EQ(pool.Capacity(), expected_capacity);
    }
  }
}

TEST(AmalgamatedBufferSlicePool, BorrowSliceFromDynamicPool) {  // NOLINT
  const SizeType reserved_pool_capacity{0};
  for (const SizeType dynamic_pool_capacity : {0, 1, 2, 5}) {
    const auto expected_capacity =
        reserved_pool_capacity + dynamic_pool_capacity;
    const ReservedPoolOptions reserved_pool_options{
        .max_slice_capacity = 1, .capacity = reserved_pool_capacity};
    const DynamicPoolOptions dynamic_pool_options{
        .max_slice_capacity = 1,
        .capacity = dynamic_pool_capacity,
        .borrow_cas_attempts = 1};
    const AmalgamatedPool::Options options{
        .reserved_pool_options = reserved_pool_options,
        .dynamic_pool_options = dynamic_pool_options};
    AmalgamatedPool pool{options};
    ASSERT_EQ(pool.Capacity(), dynamic_pool_capacity);
    ASSERT_EQ(pool.Size(), pool.Capacity());
    std::vector<OwnedSlicePtr> retained_slices{};
    retained_slices.reserve(dynamic_pool_capacity);
    std::unordered_set<Slice*> unique_slice_ptrs{};
    unique_slice_ptrs.reserve(dynamic_pool_capacity);
    for (SizeType i{0}; i < dynamic_pool_capacity; ++i) {
      auto result = pool.Borrow(1);
      ASSERT_TRUE(result.IsOk());
      auto slice = std::move(result.Ok());
      ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
      ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity - i - 1);
      ASSERT_EQ(pool.Size(), pool.DynamicPoolSize() + pool.ReservedPoolSize());
      ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
      ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
      ASSERT_EQ(pool.Capacity(), expected_capacity);
      unique_slice_ptrs.insert(slice.Ptr());
      retained_slices.push_back(std::move(slice));
    }
    ASSERT_EQ(unique_slice_ptrs.size(), dynamic_pool_capacity);
    for (SizeType i{0}; i < 3; ++i) {
      const auto result = pool.Borrow(1);
      ASSERT_TRUE(result.IsErr());
      ASSERT_EQ(result.Err(), BorrowError::kNoSlicesAvailable);
      ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
      ASSERT_EQ(pool.DynamicPoolSize(), 0);
      ASSERT_EQ(pool.Size(), pool.DynamicPoolSize());
      ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
      ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
      ASSERT_EQ(pool.Capacity(), expected_capacity);
    }
    for (SizeType i{0}; i < dynamic_pool_capacity; ++i) {
      auto slice = std::move(retained_slices.back());
      retained_slices.pop_back();
      unique_slice_ptrs.erase(slice.Ptr());
      const auto result = pool.Return(std::move(slice));
      ASSERT_TRUE(result.IsOk());
      ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
      ASSERT_EQ(pool.DynamicPoolSize(), i + 1);
      ASSERT_EQ(pool.Size(), pool.ReservedPoolSize() + pool.DynamicPoolSize());
      ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
      ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
      ASSERT_EQ(pool.Capacity(), expected_capacity);
    }
  }
}

// NOLINTNEXTLINE
TEST(AmalgamatedBufferSlicePool, BorrowSliceFromReservedAndDynamicPools) {
  for (const SizeType reserved_pool_capacity : {0, 1, 2, 5}) {
    for (const SizeType dynamic_pool_capacity : {0, 1, 2, 5}) {
      const auto expected_capacity =
          reserved_pool_capacity + dynamic_pool_capacity;
      const ReservedPoolOptions reserved_pool_options{
          .max_slice_capacity = 1, .capacity = reserved_pool_capacity};
      const DynamicPoolOptions dynamic_pool_options{
          .max_slice_capacity = 1,
          .capacity = dynamic_pool_capacity,
          .borrow_cas_attempts = 1};
      const AmalgamatedPool::Options options{
          .reserved_pool_options = reserved_pool_options,
          .dynamic_pool_options = dynamic_pool_options};
      AmalgamatedPool pool{options};
      ASSERT_EQ(pool.Capacity(), expected_capacity);
      ASSERT_EQ(pool.Size(), pool.Capacity());
      std::vector<OwnedSlicePtr> retained_slices{};
      retained_slices.reserve(reserved_pool_capacity + dynamic_pool_capacity);
      std::unordered_set<Slice*> unique_slice_ptrs{};
      unique_slice_ptrs.reserve(reserved_pool_capacity + dynamic_pool_capacity);
      for (SizeType i{0}; i < reserved_pool_capacity; ++i) {
        auto result = pool.Borrow(1);
        ASSERT_TRUE(result.IsOk());
        auto slice = std::move(result.Ok());
        ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity - i - 1);
        ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity);
        ASSERT_EQ(pool.Size(),
                  pool.ReservedPoolSize() + pool.DynamicPoolSize());
        ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
        ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
        ASSERT_EQ(pool.Capacity(), expected_capacity);
        unique_slice_ptrs.insert(slice.Ptr());
        retained_slices.push_back(std::move(slice));
      }
      for (SizeType i{0}; i < dynamic_pool_capacity; ++i) {
        auto result = pool.Borrow(1);
        ASSERT_TRUE(result.IsOk());
        auto slice = std::move(result.Ok());
        ASSERT_EQ(pool.ReservedPoolSize(), 0);
        ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity - i - 1);
        ASSERT_EQ(pool.Size(),
                  pool.ReservedPoolSize() + pool.DynamicPoolSize());
        ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
        ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
        ASSERT_EQ(pool.Capacity(), expected_capacity);
        unique_slice_ptrs.insert(slice.Ptr());
        retained_slices.push_back(std::move(slice));
      }
      ASSERT_EQ(unique_slice_ptrs.size(),
                reserved_pool_capacity + dynamic_pool_capacity);
      for (SizeType i{0}; i < 3; ++i) {
        const auto result = pool.Borrow(1);
        ASSERT_TRUE(result.IsErr());
        ASSERT_EQ(result.Err(), BorrowError::kNoSlicesAvailable);
        ASSERT_EQ(pool.ReservedPoolSize(), 0);
        ASSERT_EQ(pool.DynamicPoolSize(), 0);
        ASSERT_EQ(pool.Size(),
                  pool.ReservedPoolSize() + pool.DynamicPoolSize());
        ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
        ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
        ASSERT_EQ(pool.Capacity(), expected_capacity);
      }
      for (SizeType i{0}; i < dynamic_pool_capacity; ++i) {
        auto slice = std::move(retained_slices.back());
        retained_slices.pop_back();
        unique_slice_ptrs.erase(slice.Ptr());
        const auto result = pool.Return(std::move(slice));
        ASSERT_TRUE(result.IsOk());
        ASSERT_EQ(pool.ReservedPoolSize(), 0);
        ASSERT_EQ(pool.DynamicPoolSize(), i + 1);
        ASSERT_EQ(pool.Size(),
                  pool.ReservedPoolSize() + pool.DynamicPoolSize());
        ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
        ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
        ASSERT_EQ(pool.Capacity(), expected_capacity);
      }
      for (SizeType i{0}; i < reserved_pool_capacity; ++i) {
        auto slice = std::move(retained_slices.back());
        retained_slices.pop_back();
        unique_slice_ptrs.erase(slice.Ptr());
        const auto result = pool.Return(std::move(slice));
        ASSERT_TRUE(result.IsOk());
        ASSERT_EQ(pool.ReservedPoolSize(), i + 1);
        ASSERT_EQ(pool.DynamicPoolSize(), pool.DynamicPoolCapacity());
        ASSERT_EQ(pool.Size(),
                  pool.ReservedPoolSize() + pool.DynamicPoolSize());
        ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
        ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
        ASSERT_EQ(pool.Capacity(), expected_capacity);
      }
    }
  }
}

TEST(AmalgamatedBufferSlicePool, BulkReturnSlices) {  // NOLINT
  for (const SizeType reserved_pool_capacity : {0, 1, 2, 5}) {
    for (const SizeType dynamic_pool_capacity : {0, 1, 2, 5}) {
      const ReservedPoolOptions reserved_pool_options{
          .max_slice_capacity = 1, .capacity = reserved_pool_capacity};
      const DynamicPoolOptions dynamic_pool_options{
          .max_slice_capacity = 1,
          .capacity = dynamic_pool_capacity,
          .borrow_cas_attempts = 1};
      const AmalgamatedPool::Options options{
          .reserved_pool_options = reserved_pool_options,
          .dynamic_pool_options = dynamic_pool_options};
      AmalgamatedPool pool{options};
      std::vector<OwnedSlicePtr> retained_slices{};
      retained_slices.reserve(reserved_pool_capacity + dynamic_pool_capacity);
      for (SizeType i{0}; i < pool.Capacity(); ++i) {
        auto result = pool.Borrow(1);
        ASSERT_TRUE(result.IsOk());
        retained_slices.push_back(std::move(result.Ok()));
      }
      ASSERT_TRUE(pool.Empty());
      const auto slices_owned_by_other_pools =
          pool.Return(std::move(retained_slices));
      ASSERT_TRUE(slices_owned_by_other_pools.empty());
    }
  }
}

// NOLINTNEXTLINE
TEST(AmalgamatedBufferSlicePool, BulkReturnReturnsErrorWithSlicesItDoesNotOwn) {
  const ReservedPoolOptions reserved_pool_options{.max_slice_capacity = 1,
                                                  .capacity = 1};
  const DynamicPoolOptions dynamic_pool_options{
      .max_slice_capacity = 1, .capacity = 1, .borrow_cas_attempts = 1};
  const AmalgamatedPool::Options options{
      .reserved_pool_options = reserved_pool_options,
      .dynamic_pool_options = dynamic_pool_options};
  AmalgamatedPool pool_a{options};
  std::vector<OwnedSlicePtr> pool_a_slices{};
  pool_a_slices.reserve(pool_a.Capacity());
  for (SizeType i{0}; i < pool_a.Capacity(); ++i) {
    auto result = pool_a.Borrow(1);
    ASSERT_TRUE(result.IsOk());
    pool_a_slices.push_back(std::move(result.Ok()));
  }
  AmalgamatedPool pool_b{options};
  auto slices_not_owned_by_pool_b = pool_b.Return(std::move(pool_a_slices));
  ASSERT_EQ(slices_not_owned_by_pool_b.size(), pool_a.Capacity());
  const auto slices_not_owned_by_pool_a =
      pool_a.Return(std::move(slices_not_owned_by_pool_b));
  ASSERT_TRUE(slices_not_owned_by_pool_a.empty());
}

}  // namespace
