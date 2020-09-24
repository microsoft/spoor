#pragma once

#include <atomic>
#include <cassert>
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
  // Complexity: O(ceil(capacity / max_slice_capacity))
  [[nodiscard]] auto Borrow(SizeType preferred_slice_capacity)
      -> BorrowResult override;
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
  std::vector<T> buffer_;
  std::vector<UnownedSlice> pool_;
  std::vector<std::atomic_bool> borrowed_;
  std::atomic_size_t size_;
};

template <class T>
ReservedBufferSlicePool<T>::ReservedBufferSlicePool(const Options& options)
    : options_{options},
      buffer_{std::vector<T>(options.capacity)},
      pool_{[&]() -> std::vector<UnownedSlice> {
        if (options.max_slice_capacity == 0) return {};
        const auto maximally_sized_slices_size =
            options.capacity / options.max_slice_capacity;
        const auto remainder_slice_buffer_size =
            options.capacity % options.max_slice_capacity;
        const auto slices_size = maximally_sized_slices_size +
                                 (remainder_slice_buffer_size == 0 ? 0 : 1);
        std::vector<UnownedSlice> pool{};
        pool.reserve(slices_size);
        for (SizeType index{0}; index < maximally_sized_slices_size; ++index) {
          auto* buffer =
              std::next(buffer_.data(), options.max_slice_capacity * index);
          const std::span<T> unowned_buffer{buffer, options.max_slice_capacity};
          pool.push_back(UnownedSlice{unowned_buffer});
        }
        if (remainder_slice_buffer_size != 0) {
          auto* buffer = std::next(
              buffer_.data(), options.max_slice_capacity * (slices_size - 1));
          const std::span<T> unowned_buffer{buffer,
                                            remainder_slice_buffer_size};
          pool.push_back(UnownedSlice{unowned_buffer});
        }
        return pool;
      }()},
      borrowed_{std::vector<std::atomic_bool>(pool_.size())},
      size_{options.capacity} {}

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
auto ReservedBufferSlicePool<T>::Return(Slice* slice) -> ReturnRawPtrResult {
  auto* unowned_slice = static_cast<UnownedSlice*>(slice);
  const auto index = std::distance(pool_.data(), unowned_slice);
  if (index < 0 || Capacity() <= gsl::narrow_cast<SizeType>(index)) {
    return ReturnRawPtrResult::Err({});
  }
  size_ += slice->Capacity();
  borrowed_.at(index).store(false);
  return ReturnRawPtrResult::Ok({});
}

template <class T>
auto ReservedBufferSlicePool<T>::Return(OwnedSlicePtr&& owned_ptr)
    -> ReturnResult {
  if (owned_ptr.Owner() != this) return std::move(owned_ptr);
  const auto result = Return(owned_ptr.Take());
  if (result.IsOk()) return ReturnResult::Ok({});
  return std::move(owned_ptr);
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
