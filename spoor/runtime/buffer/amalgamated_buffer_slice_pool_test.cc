#include "spoor/runtime/buffer/amalgamated_buffer_slice_pool.h"

#include <gtest/gtest.h>

#include <unordered_set>
#include <vector>

#include "spoor/runtime/buffer/buffer_slice.h"
#include "spoor/runtime/buffer/dynamic_buffer_slice_pool.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "util/memory/owned_ptr.h"
#include "util/numeric.h"

namespace {

using Slice = spoor::runtime::buffer::BufferSlice<int64>;
using ValueType = Slice::ValueType;
using SizeType = Slice::SizeType;
using BufferSlicePool = spoor::runtime::buffer::BufferSlicePool<ValueType>;
using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
using DynamicPool = spoor::runtime::buffer::DynamicBufferSlicePool<ValueType>;
using ReservedPool = spoor::runtime::buffer::ReservedBufferSlicePool<ValueType>;

// TODO test bulk return

// TEST(BufferSlicePool, BorrowSliceFromReservedPool) {  // NOLINT
//   for (const SizeType reserved_pool_capacity : {0, 1, 2, 5}) {
//     const SizeType dynamic_pool_capacity{0};
//     const ReservedPool::Options reserved_pool_options{
//         .max_slice_capacity = 1, .capacity = reserved_pool_capacity};
//     const DynamicPool::Options dynamic_pool_options{
//         .max_slice_capacity = 1,
//         .capacity = dynamic_pool_capacity,
//         .borrow_cas_attempts = 1};
//     const BufferSlicePool::Options options{
//         .reserved_pool_options = reserved_pool_options,
//         .dynamic_pool_options = dynamic_pool_options};
//     BufferSlicePool pool{options};
//     ASSERT_EQ(pool.Size(), 0);
//     ASSERT_EQ(pool.Capacity(), reserved_pool_capacity);
//     std::vector<OwnedSlicePtr> retained_slices{};
//     retained_slices.reserve(reserved_pool_capacity);
//     std::unordered_set<Slice*> unique_slice_ptrs{};
//     unique_slice_ptrs.reserve(reserved_pool_capacity);
//     for (SizeType i{0}; i < reserved_pool_capacity; ++i) {
//       auto result = pool.Borrow(1);
//       ASSERT_TRUE(result.IsOk());
//       auto slice = std::move(result.Ok());
//       ASSERT_NE(slice.Ptr(), nullptr);
//       ASSERT_EQ(pool.ReservedPoolSize(), i + 1);
//       ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity);
//       ASSERT_EQ(pool.Size(), i + 1);
//       ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
//       ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
//       ASSERT_EQ(pool.Capacity(), reserved_pool_capacity);
//       unique_slice_ptrs.insert(slice.Ptr());
//       retained_slices.push_back(std::move(slice));
//     }
//     ASSERT_EQ(unique_slice_ptrs.size(), reserved_pool_capacity);
//     for (SizeType i{0}; i < 3; ++i) {
//       auto result = pool.Borrow(1);
//       ASSERT_TRUE(result.IsOk());
//       auto slice = std::move(result.Ok());
//       ASSERT_NE(slice.Ptr(), nullptr);
//       ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
//       ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity);
//       ASSERT_EQ(pool.Size(), reserved_pool_capacity);
//       ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
//       ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
//       ASSERT_EQ(pool.Capacity(), reserved_pool_capacity);
//     }
//     for (SizeType i{0}; i < reserved_pool_capacity; ++i) {
//       auto slice = std::move(retained_slices.back());
//       retained_slices.pop_back();
//       unique_slice_ptrs.erase(slice.Ptr());
//       const auto result = pool.Return({std::move(slice)});
//       ASSERT_EQ(result.size(), 0);
//       ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity - i - 1);
//       ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity);
//       ASSERT_EQ(pool.Size(), reserved_pool_capacity - i - 1);
//       ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
//       ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
//       ASSERT_EQ(pool.Capacity(), reserved_pool_capacity);
//     }
//   }
// }
// 
// TEST(BufferSlicePool, BorrowSliceFromDynamicPool) {  // NOLINT
//   const SizeType reserved_pool_capacity{0};
//   for (const SizeType dynamic_pool_capacity : {0, 1, 2, 5}) {
//     const BufferSlice::Options slice_options{.capacity = 0};
//     const BufferSlicePool::Options pool_options{
//         .reserved_pool_capacity = reserved_pool_capacity,
//         .dynamic_pool_capacity = dynamic_pool_capacity};
//     BufferSlicePool pool{pool_options, slice_options};
//     ASSERT_EQ(pool.Size(), reserved_pool_capacity);
//     ASSERT_EQ(pool.Capacity(), dynamic_pool_capacity);
//     std::vector<OwnedSlicePtr> retained_slices{};
//     retained_slices.reserve(dynamic_pool_capacity);
//     std::unordered_set<BufferSlice*> unique_slice_ptrs{};
//     unique_slice_ptrs.reserve(dynamic_pool_capacity);
//     for (SizeType i{0}; i < dynamic_pool_capacity; ++i) {
//       auto slice = pool.Borrow();
//       ASSERT_NE(slice.Ptr(), nullptr);
//       ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
//       ASSERT_EQ(pool.DynamicPoolSize(), i + 1);
//       ASSERT_EQ(pool.Size(), i + 1);
//       ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
//       ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
//       ASSERT_EQ(pool.Capacity(), dynamic_pool_capacity);
//       unique_slice_ptrs.insert(slice.Ptr());
//       retained_slices.push_back(std::move(slice));
//     }
//     ASSERT_EQ(unique_slice_ptrs.size(), dynamic_pool_capacity);
//     for (SizeType i{0}; i < 3; ++i) {
//       const auto slice = pool.Borrow();
//       ASSERT_EQ(slice.Ptr(), nullptr);
//       ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
//       ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity);
//       ASSERT_EQ(pool.Size(), dynamic_pool_capacity);
//       ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
//       ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
//       ASSERT_EQ(pool.Capacity(), dynamic_pool_capacity);
//     }
//     for (SizeType i{0}; i < dynamic_pool_capacity; ++i) {
//       auto slice = std::move(retained_slices.back());
//       retained_slices.pop_back();
//       unique_slice_ptrs.erase(slice.Ptr());
//       const auto result = pool.Return(std::move(slice));
//       ASSERT_TRUE(result.IsOk());
//       ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
//       ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity - i - 1);
//       ASSERT_EQ(pool.Size(), dynamic_pool_capacity - i - 1);
//       ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
//       ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
//       ASSERT_EQ(pool.Capacity(), dynamic_pool_capacity);
//     }
//   }
// }
//
// TEST(BufferSlicePool, BorrowSliceFromReservedAndDynamicPools) {  // NOLINT
//   for (const SizeType reserved_pool_capacity : {0, 1, 2, 5}) {
//     for (const SizeType dynamic_pool_capacity : {0, 1, 2, 5}) {
//       const BufferSlice::Options slice_options{.capacity = 0};
//       const BufferSlicePool::Options pool_options{
//           .reserved_pool_capacity = reserved_pool_capacity,
//           .dynamic_pool_capacity = dynamic_pool_capacity};
//       BufferSlicePool pool{pool_options, slice_options};
//       ASSERT_EQ(pool.Size(), 0);
//       const auto expected_capacity =
//           reserved_pool_capacity + dynamic_pool_capacity;
//       ASSERT_EQ(pool.Capacity(), expected_capacity);
//       std::vector<OwnedSlicePtr> retained_slices{};
//       retained_slices.reserve(reserved_pool_capacity +
//       dynamic_pool_capacity); std::unordered_set<BufferSlice*>
//       unique_slice_ptrs{}; unique_slice_ptrs.reserve(reserved_pool_capacity +
//       dynamic_pool_capacity); for (SizeType i{0}; i < reserved_pool_capacity;
//       ++i) {
//         auto slice = pool.Borrow();
//         ASSERT_NE(slice.Ptr(), nullptr);
//         ASSERT_EQ(pool.ReservedPoolSize(), i + 1);
//         ASSERT_EQ(pool.DynamicPoolSize(), 0);
//         ASSERT_EQ(pool.Size(), i + 1);
//         ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
//         ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
//         ASSERT_EQ(pool.Capacity(), expected_capacity);
//         unique_slice_ptrs.insert(slice.Ptr());
//         retained_slices.push_back(std::move(slice));
//       }
//       for (SizeType i{0}; i < dynamic_pool_capacity; ++i) {
//         auto slice = pool.Borrow();
//         ASSERT_NE(slice.Ptr(), nullptr);
//         ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
//         ASSERT_EQ(pool.DynamicPoolSize(), i + 1);
//         ASSERT_EQ(pool.Size(), reserved_pool_capacity + i + 1);
//         ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
//         ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
//         ASSERT_EQ(pool.Capacity(), expected_capacity);
//         unique_slice_ptrs.insert(slice.Ptr());
//         retained_slices.push_back(std::move(slice));
//       }
//       ASSERT_EQ(unique_slice_ptrs.size(),
//                 reserved_pool_capacity + dynamic_pool_capacity);
//       for (SizeType i{0}; i < 3; ++i) {
//         const auto slice = pool.Borrow();
//         ASSERT_EQ(slice.Ptr(), nullptr);
//         ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
//         ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity);
//         ASSERT_EQ(pool.Size(), reserved_pool_capacity +
//         dynamic_pool_capacity); ASSERT_EQ(pool.ReservedPoolCapacity(),
//         reserved_pool_capacity); ASSERT_EQ(pool.DynamicPoolCapacity(),
//         dynamic_pool_capacity); ASSERT_EQ(pool.Capacity(),
//         expected_capacity);
//       }
//       for (SizeType i{0}; i < dynamic_pool_capacity; ++i) {
//         auto slice = std::move(retained_slices.back());
//         retained_slices.pop_back();
//         unique_slice_ptrs.erase(slice.Ptr());
//         const auto result = pool.Return(std::move(slice));
//         ASSERT_TRUE(result.IsOk());
//         ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity);
//         ASSERT_EQ(pool.DynamicPoolSize(), dynamic_pool_capacity - i - 1);
//         ASSERT_EQ(pool.Size(),
//                   reserved_pool_capacity + dynamic_pool_capacity - i - 1);
//         ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
//         ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
//         ASSERT_EQ(pool.Capacity(),
//                   reserved_pool_capacity + dynamic_pool_capacity);
//       }
//       for (SizeType i{0}; i < reserved_pool_capacity; ++i) {
//         auto slice = std::move(retained_slices.back());
//         retained_slices.pop_back();
//         unique_slice_ptrs.erase(slice.Ptr());
//         const auto result = pool.Return(std::move(slice));
//         ASSERT_TRUE(result.IsOk());
//         ASSERT_EQ(pool.ReservedPoolSize(), reserved_pool_capacity - i - 1);
//         ASSERT_EQ(pool.DynamicPoolSize(), 0);
//         ASSERT_EQ(pool.Size(), reserved_pool_capacity - i - 1);
//         ASSERT_EQ(pool.ReservedPoolCapacity(), reserved_pool_capacity);
//         ASSERT_EQ(pool.DynamicPoolCapacity(), dynamic_pool_capacity);
//         ASSERT_EQ(pool.Capacity(),
//                   reserved_pool_capacity + dynamic_pool_capacity);
//       }
//     }
//   }
// }

}  // namespace
