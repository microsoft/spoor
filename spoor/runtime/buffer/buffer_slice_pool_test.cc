#include "spoor/runtime/buffer/buffer_slice_pool.h"

#include <gtest/gtest.h>

#include <unordered_set>
#include <vector>

#include "spoor/runtime/buffer/buffer_slice.h"
#include "util/memory.h"
#include "util/numeric.h"

namespace {

using BufferSlice = spoor::runtime::buffer::BufferSlice<int64>;
using ValueType = BufferSlice::ValueType;
using BufferSlicePool = spoor::runtime::buffer::BufferSlicePool<ValueType>;
using ContiguousMemory = spoor::runtime::buffer::ContiguousMemory<ValueType>;
using SizeType = BufferSlice::SizeType;
using util::memory::OwnedPtr;

TEST(BufferSlicePool, BorrowSliceFromReservedPool) {  // NOLINT
  for (const SizeType reserved_pool_capacity : {0, 1, 5}) {
    const BufferSlice::Options slice_options{.capacity = 0};
    const BufferSlicePool::Options pool_options{
        .reserved_pool_capacity = reserved_pool_capacity,
        .dynamic_pool_capacity = 0};
    BufferSlicePool pool{pool_options, slice_options};
    ASSERT_EQ(pool.Size(), 0);
    ASSERT_EQ(pool.Capacity(), reserved_pool_capacity);
    std::vector<OwnedPtr<BufferSlice>> retained_slices{};
    retained_slices.reserve(reserved_pool_capacity);
    std::unordered_set<BufferSlice*> unique_slice_ptrs{};
    unique_slice_ptrs.reserve(reserved_pool_capacity);
    for (SizeType i{0}; i < reserved_pool_capacity; ++i) {
      auto slice = pool.Borrow();
      ASSERT_NE(slice.Ptr(), nullptr);
      ASSERT_EQ(pool.Size(), i + 1);
      ASSERT_EQ(pool.Capacity(), reserved_pool_capacity);
      unique_slice_ptrs.insert(slice.Ptr());
      retained_slices.push_back(std::move(slice));
    }
    ASSERT_EQ(unique_slice_ptrs.size(), reserved_pool_capacity);
    for (SizeType i{0}; i < 3; ++i) {
      const auto slice = pool.Borrow();
      ASSERT_EQ(slice.Ptr(), nullptr);
      ASSERT_EQ(pool.Size(), reserved_pool_capacity);
      ASSERT_EQ(pool.Capacity(), reserved_pool_capacity);
    }
    // pool.Return(slices.begin(), slices.end());
  }
}

TEST(BufferSlicePool, BorrowSliceFromDynamicPool) {  // NOLINT
  for (const SizeType dynamic_pool_capacity : {0, 1, 5}) {
    const BufferSlice::Options slice_options{.capacity = 0};
    const BufferSlicePool::Options pool_options{
        .reserved_pool_capacity = 0,
        .dynamic_pool_capacity = dynamic_pool_capacity};
    BufferSlicePool pool{pool_options, slice_options};
    ASSERT_EQ(pool.Size(), 0);
    ASSERT_EQ(pool.Capacity(), dynamic_pool_capacity);
    std::vector<OwnedPtr<BufferSlice>> retained_slices{};
    retained_slices.reserve(dynamic_pool_capacity);
    std::unordered_set<BufferSlice*> unique_slice_ptrs{};
    unique_slice_ptrs.reserve(dynamic_pool_capacity);
    for (SizeType i{0}; i < dynamic_pool_capacity; ++i) {
      auto slice = pool.Borrow();
      ASSERT_NE(slice.Ptr(), nullptr);
      ASSERT_EQ(pool.Size(), i + 1);
      ASSERT_EQ(pool.Capacity(), dynamic_pool_capacity);
      unique_slice_ptrs.insert(slice.Ptr());
      retained_slices.push_back(std::move(slice));
    }
    ASSERT_EQ(unique_slice_ptrs.size(), dynamic_pool_capacity);
    for (SizeType i{0}; i < 3; ++i) {
      const auto slice = pool.Borrow();
      ASSERT_EQ(slice.Ptr(), nullptr);
      ASSERT_EQ(pool.Size(), dynamic_pool_capacity);
      ASSERT_EQ(pool.Capacity(), dynamic_pool_capacity);
    }
    // pool.Return(slices.begin(), slices.end());
  }
}

TEST(BufferSlicePool, BorrowSliceFromReservedAndDynamicPools) {  // NOLINT
  for (const SizeType reserved_pool_capacity : {0, 1, 5}) {
    for (const SizeType dynamic_pool_capacity : {0, 1, 5}) {
      const BufferSlice::Options slice_options{.capacity = 0};
      const BufferSlicePool::Options pool_options{
          .reserved_pool_capacity = reserved_pool_capacity,
          .dynamic_pool_capacity = dynamic_pool_capacity};
      BufferSlicePool pool{pool_options, slice_options};
      ASSERT_EQ(pool.Size(), 0);
      const auto expected_capacity =
          reserved_pool_capacity + dynamic_pool_capacity;
      ASSERT_EQ(pool.Capacity(), expected_capacity);
      std::vector<OwnedPtr<BufferSlice>> retained_slices{};
      retained_slices.reserve(reserved_pool_capacity + dynamic_pool_capacity);
      std::unordered_set<BufferSlice*> unique_slice_ptrs{};
      unique_slice_ptrs.reserve(reserved_pool_capacity + dynamic_pool_capacity);
      for (SizeType i{0}; i < reserved_pool_capacity; ++i) {
        auto slice = pool.Borrow();
        ASSERT_NE(slice.Ptr(), nullptr);
        ASSERT_EQ(pool.ReservedPoolSize(), i + 1);
        ASSERT_EQ(pool.DynamicPoolSize(), 0);
        ASSERT_EQ(pool.Size(), i + 1);
        ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
        ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
        ASSERT_EQ(pool.Capacity(), expected_capacity);
        unique_slice_ptrs.insert(slice.Ptr());
        retained_slices.push_back(std::move(slice));
      }
      for (SizeType i{0}; i < dynamic_pool_capacity; ++i) {
        auto slice = pool.Borrow();
        ASSERT_NE(slice.Ptr(), nullptr);
        ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
        ASSERT_EQ(pool.DynamicPoolSize(), i + 1);
        ASSERT_EQ(pool.Size(), reserved_pool_capacity + i + 1);
        ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
        ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
        ASSERT_EQ(pool.Capacity(), expected_capacity);
        unique_slice_ptrs.insert(slice.Ptr());
        retained_slices.push_back(std::move(slice));
      }
      ASSERT_EQ(unique_slice_ptrs.size(),
                reserved_pool_capacity + dynamic_pool_capacity);
      for (SizeType i{0}; i < 3; ++i) {
        const auto slice = pool.Borrow();
        ASSERT_EQ(slice.Ptr(), nullptr);
        ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
        ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity);
        ASSERT_EQ(pool.Size(), reserved_pool_capacity + dynamic_pool_capacity);
        ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
        ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
        ASSERT_EQ(pool.Capacity(), expected_capacity);
      }
      // pool.Return(slices.begin(), slices.end());
    }
  }
}

// TEST(BufferSlicePool, AssertsIfSlicesAreNotReturned) {  // NOLINT
//   const SizeType reserved_pool_capacity{3};
//   const SizeType dynamic_pool_capacity{3};
//   const BufferSlice::Options slice_options{0};
//   const BufferSlicePool::Options pool_options{reserved_pool_capacity,
//   dynamic_pool_capacity}; BufferSlicePool pool{pool_options, slice_options};
//   for (SizeType i{0}; i < reserved_pool_capacity + dynamic_pool_capacity;
//   ++i) {
//     auto* slice = pool.BorrowSlice(); // NOLINT(unused-result)
//     ASSERT_NE(slice, nullptr);
//     ASSERT_EQ(pool.BorrowedSize(), i + 1);
//   }
//   ASSERT_DEATH_IF_SUPPORTED(pool.~BufferSlicePool(), "");
// }

}  // namespace
