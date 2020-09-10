#include "spoor/runtime/buffer/buffer_slice_pool.h"

#include <gtest/gtest.h>

#include <future>
#include <limits>
#include <memory>
#include <thread>
#include <vector>

#include "spoor/runtime/buffer/amalgamated_buffer_slice_pool.h"
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
using BorrowError = BufferSlicePool::BorrowError;
using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
using DynamicPool = spoor::runtime::buffer::DynamicBufferSlicePool<ValueType>;
using ReservedPool = spoor::runtime::buffer::ReservedBufferSlicePool<ValueType>;
using AmalgamatedPool =
    spoor::runtime::buffer::AmalgamatedBufferSlicePool<ValueType>;

struct Options {
  SizeType max_slice_capacity;
  SizeType capacity;
  SizeType borrow_cas_attempts;
};

auto Pools(const Options options)
    -> std::vector<std::unique_ptr<BufferSlicePool>> {
  const typename DynamicPool::Options dynamic_pool_options{
      .max_slice_capacity = options.max_slice_capacity,
      .capacity = options.capacity,
      .borrow_cas_attempts = options.borrow_cas_attempts};
  const typename ReservedPool::Options reserved_pool_options{
      .max_slice_capacity = options.max_slice_capacity,
      .capacity = options.capacity};
  std::vector<typename AmalgamatedPool::Options> amalgamated_pool_options{
      {.reserved_pool_options = {.max_slice_capacity =
                                     options.max_slice_capacity,
                                 .capacity = options.capacity},
       .dynamic_pool_options = {.max_slice_capacity = 0,
                                .capacity = 0,
                                .borrow_cas_attempts =
                                    options.borrow_cas_attempts}},
      {.reserved_pool_options = {.max_slice_capacity = 0, .capacity = 0},
       .dynamic_pool_options = {.max_slice_capacity =
                                    options.max_slice_capacity,
                                .capacity = options.capacity,
                                .borrow_cas_attempts =
                                    options.borrow_cas_attempts}},
      {.reserved_pool_options = {.max_slice_capacity =
                                     options.max_slice_capacity,
                                 .capacity = options.max_slice_capacity *
                                             (options.capacity /
                                              options.max_slice_capacity)},
       .dynamic_pool_options =
           {.max_slice_capacity = options.max_slice_capacity,
            .capacity = options.capacity % options.max_slice_capacity,
            .borrow_cas_attempts = options.borrow_cas_attempts}},
  };
  std::vector<std::unique_ptr<BufferSlicePool>> pools{};
  pools.reserve(3 + amalgamated_pool_options.size());
  pools.push_back(std::make_unique<DynamicPool>(dynamic_pool_options));
  pools.push_back(std::make_unique<ReservedPool>(reserved_pool_options));
  for (const auto options : amalgamated_pool_options) {
    pools.push_back(std::make_unique<AmalgamatedPool>(options));
  }
  return pools;
}

TEST(BufferSlicePool, ReturnSlicePointer) {  // NOLINT
  const SizeType capacity{1};
  const Options options{.max_slice_capacity = capacity,
                        .capacity = capacity,
                        .borrow_cas_attempts = 1};
  for (const auto& pool : Pools(options)) {
    ASSERT_EQ(pool->Size(), capacity);
    auto borrow_result = pool->Borrow(capacity);
    ASSERT_TRUE(borrow_result.IsOk());
    ASSERT_EQ(pool->Size(), pool->Capacity() - capacity);
    auto slice = std::move(borrow_result.Ok());
    auto* slice_ptr = slice.Take();
    auto return_result = pool->Return(slice_ptr);
    ASSERT_TRUE(return_result.IsOk());
    ASSERT_EQ(pool->Size(), capacity);
  };
}

TEST(BufferSlicePool, ReturnOwnedSlicePtr) {  // NOLINT
  const SizeType capacity{1};
  const Options options{.max_slice_capacity = capacity,
                        .capacity = capacity,
                        .borrow_cas_attempts = 1};
  for (const auto& pool : Pools(options)) {
    ASSERT_EQ(pool->Size(), capacity);
    auto borrow_result = pool->Borrow(capacity);
    ASSERT_TRUE(borrow_result.IsOk());
    ASSERT_EQ(pool->Size(), pool->Capacity() - capacity);
    auto slice = std::move(borrow_result.Ok());
    auto return_result = pool->Return(std::move(slice));
    ASSERT_TRUE(return_result.IsOk());
    ASSERT_EQ(pool->Size(), capacity);
  }
}

TEST(BufferSlicePool, Size) {  // NOLINT
  for (const SizeType slices_size : {0, 1, 2, 5, 10}) {
    for (const SizeType slice_capacity : {1, 2, 5, 10}) {
      const auto capacity = slices_size * slice_capacity;
      const Options options{.max_slice_capacity = slice_capacity,
                            .capacity = capacity,
                            .borrow_cas_attempts = 1};
      for (const auto& pool : Pools(options)) {
        std::vector<OwnedSlicePtr> retained_slices{};
        retained_slices.reserve(slices_size);
        for (SizeType i{0}; i < slices_size; ++i) {
          auto result = pool->Borrow(slice_capacity);
          ASSERT_TRUE(result.IsOk());
          ASSERT_EQ(pool->Size(), capacity - (i + 1) * slice_capacity);
          auto slice = std::move(result.Ok());
          retained_slices.push_back(std::move(slice));
        }
        for (auto i{0}; i < 3; ++i) {
          auto result = pool->Borrow(slice_capacity);
          ASSERT_TRUE(result.IsErr());
          ASSERT_EQ(result.Err(), BorrowError::kNoSlicesAvailable);
          ASSERT_EQ(pool->Size(), 0);
        }
        SizeType expected_size{0};
        for (auto& slice : retained_slices) {
          ASSERT_EQ(pool->Size(), expected_size);
          expected_size += slice->Capacity();
          pool->Return(std::move(slice));
          ASSERT_EQ(pool->Size(), expected_size);
        }
      }
    }
  }
}

TEST(BufferSlicePool, Capacity) {  // NOLINT
  for (const SizeType slices_size : {0, 1, 2, 5, 10}) {
    for (const SizeType slice_capacity : {1, 2, 5, 10}) {
      const auto capacity = slices_size * slice_capacity;
      const Options options{.max_slice_capacity = slice_capacity,
                            .capacity = capacity,
                            .borrow_cas_attempts = 1};
      for (const auto& pool : Pools(options)) {
        std::vector<OwnedSlicePtr> retained_slices{};
        retained_slices.reserve(slices_size);
        for (SizeType i{0}; i < slices_size; ++i) {
          auto result = pool->Borrow(slice_capacity);
          ASSERT_TRUE(result.IsOk());
          ASSERT_EQ(pool->Capacity(), capacity);
          auto slice = std::move(result.Ok());
          retained_slices.push_back(std::move(slice));
        }
        for (auto i{0}; i < 3; ++i) {
          auto result = pool->Borrow(slice_capacity);
          ASSERT_TRUE(result.IsErr());
          ASSERT_EQ(result.Err(), BorrowError::kNoSlicesAvailable);
          ASSERT_EQ(pool->Capacity(), capacity);
        }
        for (auto& slice : retained_slices) {
          pool->Return(std::move(slice));
          ASSERT_EQ(pool->Capacity(), capacity);
        }
      }
    }
  }
}

TEST(BufferSlicePool, Empty) {  // NOLINT
  for (const SizeType slices_size : {0, 1, 2, 5, 10}) {
    for (const SizeType slice_capacity : {1, 2, 5, 10}) {
      const auto capacity = slices_size * slice_capacity;
      const Options options{.max_slice_capacity = slice_capacity,
                            .capacity = capacity,
                            .borrow_cas_attempts = 1};
      for (const auto& pool : Pools(options)) {
        std::vector<OwnedSlicePtr> retained_slices{};
        retained_slices.reserve(slices_size);
        for (SizeType i{0}; i < slices_size; ++i) {
          ASSERT_FALSE(pool->Empty());
          auto result = pool->Borrow(slice_capacity);
          ASSERT_TRUE(result.IsOk());
          auto slice = std::move(result.Ok());
          retained_slices.push_back(std::move(slice));
        }
        for (auto i{0}; i < 3; ++i) {
          ASSERT_TRUE(pool->Empty());
          auto result = pool->Borrow(slice_capacity);
          ASSERT_TRUE(result.IsErr());
          ASSERT_EQ(result.Err(), BorrowError::kNoSlicesAvailable);
        }
        ASSERT_TRUE(pool->Empty());
        for (auto& slice : retained_slices) {
          pool->Return(std::move(slice));
          ASSERT_FALSE(pool->Empty());
        }
      }
    }
  }
}

TEST(BufferSlicePool, Full) {  // NOLINT
  for (const SizeType slices_size : {0, 1, 2, 5, 10}) {
    for (const SizeType slice_capacity : {1, 2, 5, 10}) {
      const auto capacity = slices_size * slice_capacity;
      const Options options{.max_slice_capacity = slice_capacity,
                            .capacity = capacity,
                            .borrow_cas_attempts = 1};
      for (const auto& pool : Pools(options)) {
        ASSERT_TRUE(pool->Full());
        std::vector<OwnedSlicePtr> retained_slices{};
        retained_slices.reserve(slices_size);
        for (SizeType i{0}; i < slices_size; ++i) {
          auto result = pool->Borrow(slice_capacity);
          ASSERT_TRUE(result.IsOk());
          ASSERT_FALSE(pool->Full());
          auto slice = std::move(result.Ok());
          retained_slices.push_back(std::move(slice));
        }
        if (pool->Capacity() == 0) {
          ASSERT_TRUE(pool->Full());
        } else {
          ASSERT_FALSE(pool->Full());
        }
        for (auto i{0}; i < 3; ++i) {
          auto result = pool->Borrow(slice_capacity);
          ASSERT_TRUE(result.IsErr());
          ASSERT_EQ(result.Err(), BorrowError::kNoSlicesAvailable);
          if (pool->Capacity() == 0) {
            ASSERT_TRUE(pool->Full());
          } else {
            ASSERT_FALSE(pool->Full());
          }
        }
        for (auto& slice : retained_slices) {
          ASSERT_FALSE(pool->Full());
          pool->Return(std::move(slice));
        }
        ASSERT_TRUE(pool->Full());
      }
    }
  }
}

TEST(BufferSlicePool, MultithreadedBorrow) {  // NOLINT
  const SizeType slice_capacity{1};
  const SizeType thread_size{100};
  const SizeType extra_threads_size{2};
  const Options options{
      .max_slice_capacity = slice_capacity,
      .capacity = slice_capacity * thread_size,
      .borrow_cas_attempts = std::numeric_limits<SizeType>::max()};
  for (const auto& pool : Pools(options)) {
    const auto borrow = [&]() -> BufferSlicePool::BorrowResult {
      auto result = pool->Borrow(slice_capacity);
      if (result.IsOk()) return std::move(result.Ok());
      return result.Err();
    };
    std::vector<std::future<BufferSlicePool::BorrowResult>> futures;
    futures.reserve(thread_size + extra_threads_size);
    for (SizeType i{0}; i < thread_size + extra_threads_size; ++i) {
      futures.push_back(std::async(std::launch::async, borrow));
    }
    std::vector<OwnedSlicePtr> retained_slices{};
    retained_slices.reserve(thread_size);
    SizeType errors{0};
    for (auto& future : futures) {
      auto result = future.get();
      if (result.IsOk()) retained_slices.push_back(std::move(result.Ok()));
      if (result.IsErr()) {
        ASSERT_EQ(result.Err(), BorrowError::kNoSlicesAvailable);
        errors += 1;
      }
    }
    ASSERT_EQ(retained_slices.size(), thread_size);
    ASSERT_EQ(errors, extra_threads_size);
  }
}

}  // namespace
