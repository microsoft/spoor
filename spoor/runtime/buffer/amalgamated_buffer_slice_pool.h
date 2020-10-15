#pragma once

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iterator>
#include <new>
#include <optional>

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
  explicit AmalgamatedBufferSlicePool(const Options& options);
  AmalgamatedBufferSlicePool(const AmalgamatedBufferSlicePool&) = delete;
  AmalgamatedBufferSlicePool(AmalgamatedBufferSlicePool&&) noexcept = delete;
  auto operator=(const AmalgamatedBufferSlicePool&)
      -> AmalgamatedBufferSlicePool& = delete;
  auto operator=(AmalgamatedBufferSlicePool&&) noexcept
      -> AmalgamatedBufferSlicePool& = delete;
  ~AmalgamatedBufferSlicePool() = default;

  // Borrow a buffer from the reserved pool if available. Otherwise, returns a
  // buffer from the dynamic pool.
  [[nodiscard]] auto Borrow(SizeType preferred_slice_capacity)
      -> BorrowResult override;
  auto Return(OwnedSlicePtr&& slice) -> ReturnResult override;
  template <class Collection>
  auto Return(Collection&& slices) -> std::vector<OwnedSlicePtr>;

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

  auto Return(Slice* slice) -> ReturnRawPtrResult override;

 private:
  Options options_;
  ReservedPool reserved_pool_;
  DynamicPool dynamic_pool_;
};

template <class T>
AmalgamatedBufferSlicePool<T>::AmalgamatedBufferSlicePool(
    const Options& options)
    : options_{options},
      reserved_pool_{ReservedPool{options.reserved_pool_options}},
      dynamic_pool_{DynamicPool{options.dynamic_pool_options}} {}

template <class T>
auto AmalgamatedBufferSlicePool<T>::Borrow(
    const SizeType preferred_slice_capacity) -> BorrowResult {
  auto reserved_pool_slice_result =
      reserved_pool_.Borrow(preferred_slice_capacity);
  if (reserved_pool_slice_result.IsOk()) {
    return std::move(reserved_pool_slice_result);
  }
  auto dynamic_pool_slice_result =
      dynamic_pool_.Borrow(preferred_slice_capacity);
  return std::move(dynamic_pool_slice_result);
}

template <class T>
auto AmalgamatedBufferSlicePool<T>::Return(Slice * /*unused*/)
    -> ReturnRawPtrResult {
  return ReturnRawPtrResult::Err({});
}

template <class T>
auto AmalgamatedBufferSlicePool<T>::Return(OwnedSlicePtr&& slice)
    -> ReturnResult {
  auto owner = slice.Owner();
  if (owner != &reserved_pool_ && owner != &dynamic_pool_) {
    return std::move(slice);
  }
  auto result = owner->Return(std::move(slice));
  if (result.IsOk()) return ReturnResult::Ok({});
  return std::move(result.Err());
}

template <class T>
template <class Collection>
auto AmalgamatedBufferSlicePool<T>::Return(Collection&& slices)
    -> std::vector<OwnedSlicePtr> {
  std::vector<OwnedSlicePtr> slices_owned_by_other_pools{};
  for (auto& slice : slices) {
    auto result = Return(std::move(slice));
    if (result.IsErr()) {
      slices_owned_by_other_pools.push_back(std::move(result.Err()));
    }
  }
  return slices_owned_by_other_pools;
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::ReservedPoolSize() const
    -> SizeType {
  return reserved_pool_.Size();
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::DynamicPoolSize() const
    -> SizeType {
  return dynamic_pool_.Size();
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::Size() const -> SizeType {
  return ReservedPoolSize() + DynamicPoolSize();
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::ReservedPoolCapacity() const
    -> SizeType {
  return reserved_pool_.Capacity();
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::DynamicPoolCapacity() const
    -> SizeType {
  return dynamic_pool_.Capacity();
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::Capacity() const -> SizeType {
  return ReservedPoolCapacity() + DynamicPoolCapacity();
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::ReservedPoolEmpty() const
    -> bool {
  return reserved_pool_.Empty();
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::DynamicPoolEmpty() const -> bool {
  return dynamic_pool_.Empty();
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::Empty() const -> bool {
  return ReservedPoolEmpty() && DynamicPoolEmpty();
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::ReservedPoolFull() const -> bool {
  return reserved_pool_.Full();
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::DynamicPoolFull() const -> bool {
  return dynamic_pool_.Full();
}

template <class T>
constexpr auto AmalgamatedBufferSlicePool<T>::Full() const -> bool {
  return ReservedPoolFull() && DynamicPoolFull();
}

}  // namespace spoor::runtime::buffer
