#pragma once

#include <utility>

#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "spoor/runtime/buffer/circular_buffer.h"
#include "spoor/runtime/buffer/dynamic_buffer_slice_pool.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "util/memory/owned_ptr.h"
#include "util/result.h"

namespace spoor::runtime::buffer {

template <class T>
class AmalgamatedBufferSlicePool final : public BufferSlicePool<T> {
 public:
  using ValueType = T;
  using Slice = CircularBuffer<T>;
  using SizeType = typename Slice::SizeType;
  using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
  using BorrowError = typename BufferSlicePool<T>::BorrowError;
  using BorrowResult = util::result::Result<OwnedSlicePtr, BorrowError>;
  using ReturnResult = typename util::memory::PtrOwner<Slice>::Result;
  using ReservedPool = ReservedBufferSlicePool<T>;
  using DynamicPool = DynamicBufferSlicePool<T>;

  struct Options {
    typename ReservedPool::Options reserved_pool_options;
    typename DynamicPool::Options dynamic_pool_options;
  };

  AmalgamatedBufferSlicePool() = delete;
  constexpr explicit AmalgamatedBufferSlicePool(const Options& options);
  AmalgamatedBufferSlicePool(const AmalgamatedBufferSlicePool&) = delete;
  AmalgamatedBufferSlicePool(AmalgamatedBufferSlicePool&&) noexcept = delete;
  auto operator=(const AmalgamatedBufferSlicePool&)
      -> AmalgamatedBufferSlicePool& = delete;
  auto operator=(AmalgamatedBufferSlicePool&&) noexcept
      -> AmalgamatedBufferSlicePool& = delete;
  constexpr ~AmalgamatedBufferSlicePool() = default;

  // Borrow a buffer from the reserved pool if available. Otherwise, borrow a
  // buffer from the dynamic pool if available.
  [[nodiscard]] constexpr auto Borrow(SizeType preferred_slice_capacity)
      -> BorrowResult override;
  constexpr auto Return(OwnedSlicePtr&& slice) -> ReturnResult override;
  template <class Collection>
  constexpr auto Return(Collection&& slices) -> std::vector<OwnedSlicePtr>;

  [[nodiscard]] constexpr auto ReservedPoolSize() const -> SizeType;
  [[nodiscard]] constexpr auto DynamicPoolSize() const -> SizeType;
  [[nodiscard]] constexpr auto Size() const -> SizeType override;
  [[nodiscard]] constexpr auto ReservedPoolCapacity() const -> SizeType;
  [[nodiscard]] constexpr auto DynamicPoolCapacity() const -> SizeType;
  [[nodiscard]] constexpr auto Capacity() const -> SizeType override;
  [[nodiscard]] constexpr auto ReservedPoolEmpty() const -> bool;
  [[nodiscard]] constexpr auto DynamicPoolEmpty() const -> bool;
  [[nodiscard]] constexpr auto Empty() const -> bool override;
  [[nodiscard]] constexpr auto ReservedPoolFull() const -> bool;
  [[nodiscard]] constexpr auto DynamicPoolFull() const -> bool;
  [[nodiscard]] constexpr auto Full() const -> bool override;

 protected:
  using ReturnRawPtrResult =
      typename util::memory::PtrOwner<Slice>::ReturnRawPtrResult;

  constexpr auto Return(Slice* slice) -> ReturnRawPtrResult override;

 private:
  Options options_;
  ReservedPool reserved_pool_;
  DynamicPool dynamic_pool_;
};

}  // namespace spoor::runtime::buffer
