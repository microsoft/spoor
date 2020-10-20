#pragma once

#include <atomic>
#include <utility>
#include <vector>

#include "gsl/gsl"
#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "spoor/runtime/buffer/circular_buffer.h"
#include "spoor/runtime/buffer/unowned_buffer_slice.h"
#include "util/memory/owned_ptr.h"
#include "util/memory/ptr_owner.h"
#include "util/result.h"

namespace spoor::runtime::buffer {

template <class T>
class ReservedBufferSlicePool final : public BufferSlicePool<T> {
 public:
  using ValueType = T;
  using Slice = CircularBuffer<T>;
  using UnownedSlice = UnownedBufferSlice<T>;
  using SizeType = typename Slice::SizeType;
  using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
  using BorrowError = typename BufferSlicePool<T>::BorrowError;
  using BorrowResult = util::result::Result<OwnedSlicePtr, BorrowError>;
  using ReturnResult = typename util::memory::PtrOwner<Slice>::Result;

  struct Options {
    SizeType max_slice_capacity;
    SizeType capacity;
  };

  ReservedBufferSlicePool() = delete;
  constexpr explicit ReservedBufferSlicePool(const Options& options);
  ReservedBufferSlicePool(const ReservedBufferSlicePool&) = delete;
  ReservedBufferSlicePool(ReservedBufferSlicePool&&) = delete;
  auto operator=(const ReservedBufferSlicePool&)
      -> ReservedBufferSlicePool& = delete;
  auto operator=(ReservedBufferSlicePool&&)
      -> ReservedBufferSlicePool& = delete;
  // Although C++20 declares `std::vector`'s destructor as `constexpr`, it is
  // not yet implemented in some versions of the STL. Therefore, this class'
  // destructor cannot be virtual.
  ~ReservedBufferSlicePool();

  // Borrow a buffer slice from the object pool with its intrinsic capacity
  // (i.e. ignores the preferred slice capacity).
  // Complexity: Lock-free O(ceil(capacity / max_slice_capacity)).
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
  std::vector<T> buffer_;
  std::vector<UnownedSlice> pool_;
  std::vector<std::atomic_bool> borrowed_;
  std::atomic_size_t size_;

  static_assert(std::atomic_bool::is_always_lock_free);
  static_assert(std::atomic_size_t::is_always_lock_free);
};

}  // namespace spoor::runtime::buffer
