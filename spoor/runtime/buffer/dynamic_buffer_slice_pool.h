#pragma once

#include <atomic>
#include <cassert>
#include <memory>

#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "spoor/runtime/buffer/circular_buffer.h"
#include "spoor/runtime/buffer/owned_buffer_slice.h"
#include "util/memory/owned_ptr.h"
#include "util/memory/ptr_owner.h"
#include "util/result.h"

namespace spoor::runtime::buffer {

template <class T>
class DynamicBufferSlicePool final : public BufferSlicePool<T> {
 public:
  using ValueType = T;
  using Slice = CircularBuffer<T>;
  using OwnedSlice = OwnedBufferSlice<T>;
  using SizeType = typename Slice::SizeType;
  using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
  using BorrowError = typename BufferSlicePool<T>::BorrowError;
  using BorrowResult = util::result::Result<OwnedSlicePtr, BorrowError>;
  using ReturnResult = typename util::memory::PtrOwner<Slice>::Result;

  struct Options {
    SizeType max_slice_capacity;
    SizeType capacity;
    SizeType borrow_cas_attempts;
  };

  DynamicBufferSlicePool() = delete;
  explicit constexpr DynamicBufferSlicePool(const Options& options);
  DynamicBufferSlicePool(const DynamicBufferSlicePool&) = delete;
  DynamicBufferSlicePool(DynamicBufferSlicePool&&) noexcept = delete;
  auto operator=(const DynamicBufferSlicePool&)
      -> DynamicBufferSlicePool& = delete;
  auto operator=(DynamicBufferSlicePool&&) noexcept
      -> DynamicBufferSlicePool& = delete;
  ~DynamicBufferSlicePool();

  // Borrow a buffer slice from the object pool whose size is the minimum of:
  // - The preferred slice size.
  // - The maximum allowed dynamic slice size.
  // - The remaining dynamic slices size.
  // Complexity:
  // - Lock-free O(borrow_cas_attempts) worst case.
  // - Lock-free O(1) with no thread contention.
  [[nodiscard]] auto Borrow(SizeType preferred_slice_capacity)
      -> BorrowResult override;
  // Return a buffer slice to the object pool.
  // Complexity: Lock-free O(1)
  auto Return(OwnedSlicePtr&& owned_ptr) -> ReturnResult override;

  [[nodiscard]] constexpr auto Size() const -> SizeType override;
  [[nodiscard]] constexpr auto Capacity() const -> SizeType override;
  [[nodiscard]] constexpr auto Empty() const -> bool override;
  [[nodiscard]] constexpr auto Full() const -> bool override;

 protected:
  using ReturnRawPtrResult =
      typename util::memory::PtrOwner<Slice>::ReturnRawPtrResult;

  auto Return(Slice* slice) -> ReturnRawPtrResult override;

 private:
  Options options_;
  std::atomic_size_t borrowed_items_size_;
};

template <class T>
constexpr DynamicBufferSlicePool<T>::DynamicBufferSlicePool(
    const Options& options)
    : options_{options}, borrowed_items_size_{0} {}

template <class T>
DynamicBufferSlicePool<T>::~DynamicBufferSlicePool() {
  assert(Full());
}

template <class T>
auto DynamicBufferSlicePool<T>::Borrow(SizeType preferred_slice_capacity)
    -> BorrowResult {
  for (SizeType attempt{0}; attempt < options_.borrow_cas_attempts; ++attempt) {
    auto borrowed_items_size = borrowed_items_size_.load();
    const auto buffer_size =
        std::min({preferred_slice_capacity, options_.max_slice_capacity,
                  Capacity(), Capacity() - borrowed_items_size});
    const auto new_borrowed_items_size = borrowed_items_size + buffer_size;
    const auto exchanged = borrowed_items_size_.compare_exchange_weak(
        borrowed_items_size, new_borrowed_items_size);
    if (exchanged) {
      if (buffer_size < 1) return BorrowError::kNoSlicesAvailable;
      auto* slice = new OwnedSlice{buffer_size};
      return OwnedSlicePtr{slice, this};
    }
  }
  return BorrowError::kCasAttemptsExhausted;
}

template <class T>
auto DynamicBufferSlicePool<T>::Return(Slice* slice) -> ReturnRawPtrResult {
  // The dynamic buffer pool does not store a table of borrowed slices,
  // therefore, this method deletes a slice regardless if it is owned by the
  // pool or not.
  borrowed_items_size_ -= slice->Capacity();
  delete slice;
  return ReturnRawPtrResult::Ok({});
}

template <class T>
auto DynamicBufferSlicePool<T>::Return(OwnedSlicePtr&& owned_ptr)
    -> ReturnResult {
  if (owned_ptr.Owner() != this) return std::move(owned_ptr);
  const auto result = Return(owned_ptr.Take());
  if (result.IsOk()) return ReturnResult::Ok({});
  return std::move(owned_ptr);
}

template <class T>
constexpr auto DynamicBufferSlicePool<T>::Size() const -> SizeType {
  return Capacity() - borrowed_items_size_;
}

template <class T>
constexpr auto DynamicBufferSlicePool<T>::Capacity() const -> SizeType {
  return options_.capacity;
}

template <class T>
constexpr auto DynamicBufferSlicePool<T>::Empty() const -> bool {
  return Size() == 0;
}

template <class T>
constexpr auto DynamicBufferSlicePool<T>::Full() const -> bool {
  return Capacity() <= Size();
}

}  // namespace spoor::runtime::buffer
