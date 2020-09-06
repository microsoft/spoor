#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_POOL_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_POOL_H_

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iterator>
#include <new>
#include <optional>

#include "gsl/pointers.h"
#include "spoor/runtime/buffer/buffer_slice.h"
#include "spoor/runtime/buffer/buffer_slice_pool_ownership_info.h"
#include "spoor/runtime/buffer/contiguous_memory.h"
#include "util/memory.h"
#include "util/result.h"

namespace spoor::runtime::buffer {

using util::memory::OwnedPtr;
using util::memory::PtrOwner;
using util::result::Result;
using util::result::Void;

template <class T>
class BufferSlicePool final
    : public PtrOwner<BufferSlice<T>, BufferSlicePoolOwnershipInfo<T>> {
 public:
  using Slice = BufferSlice<T>;
  using SliceOptions = typename Slice::Options;
  using OwnershipInfo = BufferSlicePoolOwnershipInfo<T>;
  using OwnedSlicePtr = OwnedPtr<Slice, OwnershipInfo>;
  using PtrOwnerError = typename PtrOwner<Slice, OwnershipInfo>::Error;
  using SizeType = std::size_t;
  using ValueType = T;

  struct Options {
    SizeType reserved_pool_capacity;
    SizeType dynamic_pool_capacity;
  };

  BufferSlicePool() = delete;
  BufferSlicePool(const Options& options, const SliceOptions& slice_options);
  BufferSlicePool(const BufferSlicePool&) = delete;
  BufferSlicePool(BufferSlicePool&&) = delete;
  auto operator=(const BufferSlicePool&) -> BufferSlicePool& = delete;
  auto operator=(BufferSlicePool &&) -> BufferSlicePool& = delete;
  ~BufferSlicePool();

  [[nodiscard]] auto Borrow() -> OwnedSlicePtr;

  [[nodiscard]] auto Owns(const OwnedSlicePtr& owned_slice) const
      -> bool override;
  auto Return(gsl::owner<Slice*> slice, const OwnershipInfo& ownership_info)
      -> Result<Void, PtrOwnerError> override;
  auto Return(OwnedSlicePtr&& owned_ptr)
      -> Result<Void, PtrOwnerError> override;

  [[nodiscard]] constexpr auto SliceCapacity() -> SizeType;

  [[nodiscard]] constexpr auto ReservedPoolSize() -> SizeType;
  [[nodiscard]] constexpr auto DynamicPoolSize() -> SizeType;
  [[nodiscard]] constexpr auto Size() -> SizeType;
  [[nodiscard]] constexpr auto ReservedPoolCapacity() -> SizeType;
  [[nodiscard]] constexpr auto DynamicPoolCapacity() -> SizeType;
  [[nodiscard]] constexpr auto Capacity() -> SizeType;
  [[nodiscard]] constexpr auto ReservedPoolEmpty() -> bool;
  [[nodiscard]] constexpr auto DynamicPoolEmpty() -> bool;
  [[nodiscard]] constexpr auto Empty() -> bool;

 private:
  [[nodiscard]] auto BelongsToReservedPool(
      const Slice* slice, const OwnershipInfo& ownership_info) const -> bool;
  [[nodiscard]] auto BelongsToDynamicPool(
      const OwnershipInfo& ownership_info) const -> bool;

  const Options options_;
  const SliceOptions slice_options_;
  T* buffer_;
  Slice* reserved_pool_;
  std::atomic_bool* reserved_slice_borrowed_;
  std::atomic_size_t reserved_pool_size_;
  std::atomic_size_t dynamic_pool_size_;

  static_assert(std::pointer_traits<decltype(reserved_slice_borrowed_)>::
                    element_type::is_always_lock_free);
  static_assert(decltype(reserved_pool_size_)::is_always_lock_free);
  static_assert(decltype(dynamic_pool_size_)::is_always_lock_free);
};

// TODO static asserts for constructor and destructor availability

template <class T>
BufferSlicePool<T>::BufferSlicePool(const Options& options,
                                    const SliceOptions& slice_options)
    : options_{options},
      slice_options_{slice_options},
      buffer_{static_cast<T*>(
          ::operator new(sizeof(T) * options_.reserved_pool_capacity *
                         slice_options_.capacity))},
      reserved_pool_{static_cast<Slice*>(
          ::operator new(sizeof(Slice) * options_.reserved_pool_capacity))},
      reserved_slice_borrowed_{
          new std::atomic_bool[options_.reserved_pool_capacity]},
      reserved_pool_size_{0},
      dynamic_pool_size_{0} {
  for (SizeType i{0}; i < options_.reserved_pool_capacity; ++i) {
    auto* slice = std::next(reserved_pool_, i);
    auto* buffer = std::next(buffer_, i * slice_options_.capacity);
    new (slice) Slice{slice_options, buffer};
  }
}

template <class T>
BufferSlicePool<T>::~BufferSlicePool() {
  // TODO reverse order
  for (SizeType i{0}; i < options_.reserved_pool_capacity; ++i) {
    auto* slice = std::next(reserved_pool_, i);
    slice->~Slice();
  }
  delete[] reserved_slice_borrowed_;
  ::operator delete(reserved_pool_);
  ::operator delete(buffer_);
}

template <class T>
auto BufferSlicePool<T>::Borrow() -> OwnedSlicePtr {
  for (SizeType i{0}; i < options_.reserved_pool_capacity; ++i) {
    auto* borrowed = std::next(reserved_slice_borrowed_, i);
    if (!std::atomic_exchange(borrowed, true)) {
      ++reserved_pool_size_;
      auto* reserved_pool_borrowed_flag =
          std::next(reserved_slice_borrowed_, i);
      OwnershipInfo ownership_info{reserved_pool_borrowed_flag,
                                   &reserved_pool_size_};
      auto* slice = std::next(reserved_pool_, i);
      return {slice, this, ownership_info};
    }
  }
  if (dynamic_pool_size_.load() < options_.dynamic_pool_capacity) {
    ++dynamic_pool_size_;
    auto* slice = new Slice{slice_options_};
    OwnershipInfo ownership_info{&dynamic_pool_size_};
    return {slice, this, ownership_info};
  }
  OwnershipInfo ownership_info{nullptr};
  return {nullptr, this, ownership_info};
}

template <class T>
auto BufferSlicePool<T>::Owns(const OwnedSlicePtr& owned_slice) const -> bool {
  if (owned_slice.Owner() != this) return false;
  return BelongsToReservedPool(owned_slice.Ptr(),
                               owned_slice.OwnershipInfo()) ||
         BelongsToDynamicPool(owned_slice.OwnershipInfo());
}

template <class T>
auto BufferSlicePool<T>::Return(gsl::owner<Slice*> slice,
                                const OwnershipInfo& ownership_info)
    -> Result<Void, PtrOwnerError> {
  if (BelongsToReservedPool(slice, ownership_info)) {
    auto* borrowed_flag = ownership_info.reserved_pool_borrowed_flag_.value();
    std::atomic_store(borrowed_flag, false);
    --reserved_pool_size_;
    return Result<Void, PtrOwnerError>::Ok({});
  }
  if (BelongsToDynamicPool(ownership_info)) {
    delete slice;
    --dynamic_pool_size_;
    return Result<Void, PtrOwnerError>::Ok({});
  }
  return {PtrOwnerError::kDoesNotOwnPtr};
}

template <class T>
auto BufferSlicePool<T>::Return(OwnedSlicePtr&& owned_ptr)
    -> Result<Void, PtrOwnerError> {
  auto ownership_info = owned_ptr.OwnershipInfo();
  auto ptr = owned_ptr.Ptr();

  return Return(ptr, ownership_info);
}

template <class T>
constexpr auto BufferSlicePool<T>::ReservedPoolSize() -> SizeType {
  return reserved_pool_size_.load();
}

template <class T>
constexpr auto BufferSlicePool<T>::DynamicPoolSize() -> SizeType {
  return dynamic_pool_size_.load();
}

template <class T>
constexpr auto BufferSlicePool<T>::Size() -> SizeType {
  return ReservedPoolSize() + DynamicPoolSize();
}

template <class T>
constexpr auto BufferSlicePool<T>::ReservedPoolCapacity() -> SizeType {
  return options_.reserved_pool_capacity;
}

template <class T>
constexpr auto BufferSlicePool<T>::DynamicPoolCapacity() -> SizeType {
  return options_.dynamic_pool_capacity;
}

template <class T>
constexpr auto BufferSlicePool<T>::Capacity() -> SizeType {
  return ReservedPoolCapacity() + DynamicPoolCapacity();
}

template <class T>
constexpr auto BufferSlicePool<T>::ReservedPoolEmpty() -> bool {
  return ReservedPoolSize() == 0;
}

template <class T>
constexpr auto BufferSlicePool<T>::DynamicPoolEmpty() -> bool {
  return DynamicPoolSize() == 0;
}

template <class T>
constexpr auto BufferSlicePool<T>::Empty() -> bool {
  return ReservedPoolEmpty() && DynamicPoolEmpty();
}

template <class T>
auto BufferSlicePool<T>::BelongsToReservedPool(
    const Slice* slice, const OwnershipInfo& ownership_info) const -> bool {
  const auto* reserved_pool = reserved_pool_;
  const auto distance = std::distance(reserved_pool, slice);
  return -1 < distance &&
         static_cast<SizeType>(distance) <
             options_.reserved_pool_capacity * sizeof(T) &&
         ownership_info.reserved_pool_borrowed_flag_.has_value() &&
         ownership_info.pool_size_ == &reserved_pool_size_;
}

template <class T>
auto BufferSlicePool<T>::BelongsToDynamicPool(
    const OwnershipInfo& ownership_info) const -> bool {
  return !ownership_info.reserved_pool_borrowed_flag_.has_value() &&
         ownership_info.pool_size_ == &dynamic_pool_size_;
}

}  // namespace spoor::runtime::buffer

#endif
