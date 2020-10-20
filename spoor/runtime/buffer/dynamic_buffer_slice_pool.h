#pragma once

#include <algorithm>
#include <atomic>
#include <utility>

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
  constexpr explicit DynamicBufferSlicePool(const Options& options);
  DynamicBufferSlicePool(const DynamicBufferSlicePool&) = delete;
  DynamicBufferSlicePool(DynamicBufferSlicePool&&) noexcept = delete;
  auto operator=(const DynamicBufferSlicePool&)
      -> DynamicBufferSlicePool& = delete;
  auto operator=(DynamicBufferSlicePool&&) noexcept
      -> DynamicBufferSlicePool& = delete;
  constexpr ~DynamicBufferSlicePool();

  // Borrow a buffer slice from the object pool whose size is the minimum of:
  // - The preferred slice size.
  // - The maximum allowed dynamic slice size.
  // - The remaining dynamic slices size.
  // Complexity:
  // - Lock-free O(borrow_cas_attempts) worst case.
  // - Lock-free O(1) with no thread contention.
  [[nodiscard]] constexpr auto Borrow(SizeType preferred_slice_capacity)
      -> BorrowResult override;
  // Return a buffer slice to the object pool.
  // Complexity: Lock-free O(1).
  constexpr auto Return(OwnedSlicePtr&& owned_ptr) -> ReturnResult override;

  [[nodiscard]] constexpr auto Size() const -> SizeType override;
  [[nodiscard]] constexpr auto Capacity() const -> SizeType override;
  [[nodiscard]] constexpr auto Empty() const -> bool override;
  [[nodiscard]] constexpr auto Full() const -> bool override;

 protected:
  using ReturnRawPtrResult =
      typename util::memory::PtrOwner<Slice>::ReturnRawPtrResult;

  constexpr auto Return(Slice* slice) -> ReturnRawPtrResult override;

 private:
  Options options_;
  std::atomic_size_t borrowed_items_size_;

  static_assert(std::atomic_size_t::is_always_lock_free);
};

}  // namespace spoor::runtime::buffer
