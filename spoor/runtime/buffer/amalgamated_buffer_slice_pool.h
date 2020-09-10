#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_AMALGAMATED_BUFFER_SLICE_POOL_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_AMALGAMATED_BUFFER_SLICE_POOL_H_

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iterator>
#include <new>
#include <optional>

#include "spoor/runtime/buffer/buffer_slice.h"
#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "spoor/runtime/buffer/dynamic_buffer_slice_pool.h"
#include "spoor/runtime/buffer/reserved_buffer_slice_pool.h"
#include "util/memory/owned_ptr.h"
#include "util/result.h"

namespace spoor::runtime::buffer {

template <class T>
class AmalgamatedBufferSlicePool final : public BufferSlicePool<T> {
 public:
  using ValueType = T;
  using Slice = BufferSlice<T>;
  using SizeType = typename Slice::SizeType;
  using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
  using PtrOwnerError = typename util::memory::PtrOwner<Slice>::Error;
  using BorrowError = typename BufferSlicePool<T>::BorrowError;
  using BorrowResult = util::result::Result<OwnedSlicePtr, BorrowError>;
  using ReturnResult = util::result::Result<util::result::Void, PtrOwnerError>;
  using ReservedPool = ReservedBufferSlicePool<T>;
  using DynamicPool = DynamicBufferSlicePool<T>;

  struct Options {
    typename ReservedPool::Options reserved_pool_options;
    typename DynamicPool::Options dynamic_pool_options;
  };

  AmalgamatedBufferSlicePool() = delete;
  explicit AmalgamatedBufferSlicePool(const Options& options);
  AmalgamatedBufferSlicePool(const AmalgamatedBufferSlicePool&) = delete;
  AmalgamatedBufferSlicePool(AmalgamatedBufferSlicePool&&) = delete;
  auto operator=(const AmalgamatedBufferSlicePool&)
      -> AmalgamatedBufferSlicePool& = delete;
  auto operator=(AmalgamatedBufferSlicePool &&)
      -> AmalgamatedBufferSlicePool& = delete;
  ~AmalgamatedBufferSlicePool() = default;

  // Borrow a buffer from the reserved pool if available. Otherwise, returns a
  // buffer from the dynamic pool.
  [[nodiscard]] auto Borrow(SizeType preferred_slice_capacity)
      -> BorrowResult override;
  auto Return(gsl::owner<Slice*> slice) -> ReturnResult override;
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

 private:
  const Options options_;
  ReservedPool reserved_pool_;
  DynamicPool dynamic_pool_;

  // static_assert(std::pointer_traits<decltype(reserved_slice_borrowed_)>::
  //                   element_type::is_always_lock_free);
  // static_assert(decltype(reserved_pool_size_)::is_always_lock_free);
  // static_assert(decltype(dynamic_pool_size_)::is_always_lock_free);
};

template <class T>
AmalgamatedBufferSlicePool<T>::AmalgamatedBufferSlicePool(
    const Options& options)
    : options_{options},
      reserved_pool_{ReservedPool{options_.reserved_pool_options}},
      dynamic_pool_{DynamicPool{options_.dynamic_pool_options}} {}

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
auto AmalgamatedBufferSlicePool<T>::Return(gsl::owner<Slice*> slice)
    -> ReturnResult {
  auto result = reserved_pool_.Return(slice);
  if (result.IsOk()) return ReturnResult::Ok({});
  return dynamic_pool_.Return(slice);
}

template <class T>
auto AmalgamatedBufferSlicePool<T>::Return(OwnedSlicePtr&& slice)
    -> ReturnResult {
  auto owner = slice.Owner();
  return owner->Return(std::move(slice));
}

template <class T>
template <class Collection>
auto AmalgamatedBufferSlicePool<T>::Return(Collection&& slices)
    -> std::vector<OwnedSlicePtr> {
  std::vector<OwnedSlicePtr> corrupt_slices{};
  for (auto slice : slices) {
    const auto owner = slice.Owner();
    auto result = owner->Return(slice.Ptr());
    if (result.IsOk()) continue;
    corrupt_slices.push_back(std::move(slice));
  }
  return corrupt_slices;
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

#endif
