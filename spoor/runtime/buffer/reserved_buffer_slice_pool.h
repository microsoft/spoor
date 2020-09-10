#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_RESERVED_BUFFER_SLICE_POOL_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_RESERVED_BUFFER_SLICE_POOL_H_

#include <atomic>
#include <cassert>
#include <iostream>  // TODO
#include <vector>

#include "spoor/runtime/buffer/buffer_slice.h"
#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "util/memory/owned_ptr.h"
#include "util/memory/ptr_owner.h"
#include "util/result.h"

namespace spoor::runtime::buffer {

template <class T>
class ReservedBufferSlicePool final : public BufferSlicePool<T> {
 public:
  using ValueType = T;
  using Slice = BufferSlice<T>;
  using SizeType = typename Slice::SizeType;
  using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
  using PtrOwnerError = typename util::memory::PtrOwner<Slice>::Error;
  using BorrowError = typename BufferSlicePool<T>::BorrowError;
  using BorrowResult = util::result::Result<OwnedSlicePtr, BorrowError>;
  using ReturnResult = util::result::Result<util::result::Void, PtrOwnerError>;

  struct Options {
    SizeType max_slice_capacity;
    SizeType capacity;
  };

  ReservedBufferSlicePool() = delete;
  explicit ReservedBufferSlicePool(const Options& options);
  ReservedBufferSlicePool(const ReservedBufferSlicePool&) = delete;
  ReservedBufferSlicePool(ReservedBufferSlicePool&&) = delete;
  auto operator=(const ReservedBufferSlicePool&)
      -> ReservedBufferSlicePool& = delete;
  auto operator=(ReservedBufferSlicePool &&)
      -> ReservedBufferSlicePool& = delete;
  ~ReservedBufferSlicePool();

  // Returns a buffer slice with its intrinsic capacity (i.e. ignores the
  // preferred slice capacity).
  [[nodiscard]] auto Borrow(SizeType) -> BorrowResult override;
  auto Return(gsl::owner<Slice*> slice) -> ReturnResult override;
  auto Return(OwnedSlicePtr&& owned_ptr) -> ReturnResult override;

  [[nodiscard]] constexpr auto Size() const -> SizeType override;
  [[nodiscard]] constexpr auto Capacity() const -> SizeType override;
  [[nodiscard]] constexpr auto Empty() const -> bool override;
  [[nodiscard]] constexpr auto Full() const -> bool override;

 private:
  const Options options_;
  std::vector<T> buffer_;
  std::vector<Slice> pool_;
  std::vector<std::atomic_bool> borrowed_;
  std::atomic_size_t size_;
};

template <class T>
ReservedBufferSlicePool<T>::ReservedBufferSlicePool(const Options& options)
    : options_{options},
      buffer_{std::vector<T>(options_.capacity)},
      pool_{[&]() -> std::vector<Slice> {
        if (options_.max_slice_capacity == 0) return {};
        const auto maximally_sized_slices_size =
            options_.capacity / options_.max_slice_capacity;
        const auto remainder_slice_buffer_size =
            options_.capacity % options_.max_slice_capacity;
        const auto slices_size = maximally_sized_slices_size +
                                 (remainder_slice_buffer_size == 0 ? 0 : 1);
        std::vector<Slice> pool{};
        pool.reserve(slices_size);
        for (SizeType index{0}; index < maximally_sized_slices_size; ++index) {
          const auto buffer =
              std::next(buffer_.data(), options_.max_slice_capacity * index);
          pool.push_back(Slice{buffer, options_.max_slice_capacity});
        }
        if (remainder_slice_buffer_size != 0) {
          const auto buffer = std::next(
              buffer_.data(), options_.max_slice_capacity * (slices_size - 1));
          pool.push_back(Slice{buffer, remainder_slice_buffer_size});
        }
        return pool;
      }()},
      borrowed_{std::vector<std::atomic_bool>(pool_.size())},
      size_{options_.capacity} {}

template <class T>
ReservedBufferSlicePool<T>::~ReservedBufferSlicePool() {
  assert(Full());
}

template <class T>
auto ReservedBufferSlicePool<T>::Borrow(SizeType /*unused*/) -> BorrowResult {
  for (SizeType index{0}; index < pool_.size(); ++index) {
    if (!borrowed_.at(index).exchange(true)) {
      auto* slice = &pool_.at(index);
      size_ -= slice->Capacity();
      return OwnedSlicePtr{slice, this};
    }
  }
  return BorrowError::kNoSlicesAvailable;
}

template <class T>
auto ReservedBufferSlicePool<T>::Return(gsl::owner<Slice*> slice)
    -> ReturnResult {
  const auto index = std::distance(pool_.data(), slice);
  if (index < 0 || options_.capacity <= static_cast<SizeType>(index)) {
    return PtrOwnerError::kDoesNotOwnPtr;
  }
  size_ += slice->Capacity();
  borrowed_.at(index).store(false);
  return ReturnResult::Ok({});
}

template <class T>
auto ReservedBufferSlicePool<T>::Return(OwnedSlicePtr&& owned_ptr)
    -> ReturnResult {
  if (owned_ptr.Owner() != this) return PtrOwnerError::kDoesNotOwnPtr;
  return Return(owned_ptr.Take());
}

template <class T>
constexpr auto ReservedBufferSlicePool<T>::Size() const -> SizeType {
  return size_.load();
}

template <class T>
constexpr auto ReservedBufferSlicePool<T>::Capacity() const -> SizeType {
  return options_.capacity;
}

template <class T>
constexpr auto ReservedBufferSlicePool<T>::Empty() const -> bool {
  return Size() == 0;
}

template <class T>
constexpr auto ReservedBufferSlicePool<T>::Full() const -> bool {
  return Capacity() <= Size();
}

}  // namespace spoor::runtime::buffer

#endif
