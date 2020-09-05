#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_POOL_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_POOL_H_

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <mutex>
#include <new>
#include <shared_mutex>
#include <vector>

#include "gsl/pointers.h"
#include "spoor/runtime/buffer/buffer_slice.h"
#include "spoor/runtime/buffer/contiguous_memory.h"
#include "util/memory.h"
#include "util/result.h"

namespace spoor::runtime::buffer {

using util::memory::OwnedPtr;
using util::memory::PtrOwner;
using util::result::Result;
using util::result::Void;

template <class T>
class BufferSlicePool final : public PtrOwner<BufferSlice<T>> {
 public:
  using Slice = BufferSlice<T>;
  using SliceOptions = typename Slice::Options;
  using PtrOwnerError = typename PtrOwner<Slice>::Error;
  using SizeType = std::size_t;
  using ValueType = T;

  struct Options {
    SizeType reserved_pool_capacity;
    SizeType dynamic_pool_capacity;
  };

  BufferSlicePool() = delete;
  explicit BufferSlicePool(const Options& options,
                           const SliceOptions& slice_options);
  BufferSlicePool(const BufferSlicePool&) = delete;
  BufferSlicePool(BufferSlicePool&&) = delete;
  auto operator=(const BufferSlicePool&) -> BufferSlicePool& = delete;
  auto operator=(BufferSlicePool &&) -> BufferSlicePool& = delete;
  ~BufferSlicePool();

  [[nodiscard]] auto Owns(const OwnedPtr<Slice>& owned_slice) const
      -> bool override;
  [[nodiscard]] auto Borrow() -> OwnedPtr<Slice> override;
  auto Return(Slice* slice) -> Result<Void, PtrOwnerError> override;
  auto Return(OwnedPtr<Slice>&& owned_slice)
      -> Result<Void, PtrOwnerError> override;

  // template <class Iterator>
  // requires(
  //     std::is_same_v<typename std::iterator_traits<Iterator>::value_type,
  //                  gsl::owner<BufferSlice<T>*>>) auto Return(Iterator begin,
  //                                                              Iterator end)
  //     -> void;

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
  Options options_;
  SliceOptions slice_options_;
  std::mutex mutex_;
  Slice* reserved_pool_;
  T* buffer_;
  std::vector<Slice*> available_reserved_objects_;
  SizeType dynamic_pool_size_;
};

// TODO static asserts for constructor and destructor availability

template <class T>
BufferSlicePool<T>::BufferSlicePool(const Options& options,
                                    const SliceOptions& slice_options)
    : options_{options},
      slice_options_{slice_options},
      mutex_{},
      reserved_pool_{static_cast<Slice*>(
          ::operator new(sizeof(Slice) * options_.reserved_pool_capacity))},
      buffer_{static_cast<T*>(
          ::operator new(sizeof(T) * options_.reserved_pool_capacity *
                         slice_options_.capacity))},
      available_reserved_objects_{},
      dynamic_pool_size_{0} {
  available_reserved_objects_.reserve(options.reserved_pool_capacity);
  for (SizeType i{0}; i < options_.reserved_pool_capacity; ++i) {
    auto* slice = std::next(reserved_pool_, i);
    auto* buffer = std::next(buffer_, i * slice_options_.capacity);
    new (slice) Slice{slice_options, buffer};
    available_reserved_objects_.push_back(slice);
  }
}

template <class T>
BufferSlicePool<T>::~BufferSlicePool() {
  // TODO reverse order
  for (SizeType i{0}; i < options_.reserved_pool_capacity; ++i) {
    std::next(reserved_pool_, i)->~Slice();
  }
  ::operator delete(reserved_pool_);
  ::operator delete(buffer_);
}

template <class T>
auto BufferSlicePool<T>::Owns(const OwnedPtr<Slice>& owned_slice) const
    -> bool {
  if (owned_slice.Owner() != this) return false;
  const auto distance = std::distance(reserved_pool_, owned_slice.Ptr());
  if (-1 < distance && static_cast<SizeType>(distance) <
                           options_.reserved_pool_capacity * sizeof(T)) {
    return true;
  }
  return true;  // TODO mutex and set
}

template <class T>
auto BufferSlicePool<T>::Borrow() -> OwnedPtr<Slice> {
  std::scoped_lock lock{mutex_};
  if (available_reserved_objects_.empty()) {
    if (dynamic_pool_size_ < options_.dynamic_pool_capacity) {
      auto* slice = new Slice{slice_options_};
      dynamic_pool_size_ += 1;
      return {slice, this};
    }
    return {nullptr, this};
  }
  auto slice = available_reserved_objects_.back();
  available_reserved_objects_.pop_back();
  return {slice, this};
}

template <class T>
auto BufferSlicePool<T>::Return(Slice* slice) -> Result<Void, PtrOwnerError> {
  const auto distance = std::distance(reserved_pool_, slice);
  std::scoped_lock lock{mutex_};
  if (-1 < distance && static_cast<SizeType>(distance) <
                           sizeof(T) * options_.reserved_pool_capacity) {
    slice->Clear();
    available_reserved_objects_.push_back(slice);
  } else if (slice != nullptr) {
    dynamic_pool_size_ -= 1;
    delete slice;
  }
  return {{}};  // todo set
}

template <class T>
auto BufferSlicePool<T>::Return(OwnedPtr<Slice>&& owned_slice)
    -> Result<Void, PtrOwnerError> {
  if (!Owns(owned_slice)) return {PtrOwnerError::kDoesNotOwnPtr};
  return Return(owned_slice.Take());
}

// template <class T>
// template <class Iterator>
// requires(std::is_same_v<typename std::iterator_traits<Iterator>::value_type,
//                         gsl::owner<BufferSlice<T>*>>) auto
//                         BufferSlicePool<T>::
//     Return(Iterator begin, Iterator end) -> void {
//   for (auto slice = begin; slice != end; ++slice) {
//     Return(gsl::owner<Slice*>(*slice));
//   }
// }

template <class T>
constexpr auto BufferSlicePool<T>::ReservedPoolSize() -> SizeType {
  return options_.reserved_pool_capacity - available_reserved_objects_.size();
}

template <class T>
constexpr auto BufferSlicePool<T>::DynamicPoolSize() -> SizeType {
  return dynamic_pool_size_;
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
  return available_reserved_objects_.size() == options_.reserved_pool_capacity;
}

template <class T>
constexpr auto BufferSlicePool<T>::DynamicPoolEmpty() -> bool {
  return dynamic_pool_size_ == 0;
}

template <class T>
constexpr auto BufferSlicePool<T>::Empty() -> bool {
  return ReservedPoolEmpty() && DynamicPoolEmpty();
}

}  // namespace spoor::runtime::buffer

#endif
