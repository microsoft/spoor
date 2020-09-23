#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_DYNAMIC_BUFFER_SLICE_POOL_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_DYNAMIC_BUFFER_SLICE_POOL_H_

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
  using PtrOwnerError = typename util::memory::PtrOwner<Slice>::Error;
  using BorrowError = typename BufferSlicePool<T>::BorrowError;
  using BorrowResult = util::result::Result<OwnedSlicePtr, BorrowError>;
  using ReturnResult = util::result::Result<util::result::None, PtrOwnerError>;

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

  // Returns a buffer slice whose size is the minimum of:
  // - The preferred slice size.
  // - The maximum allowed dynamic slice size.
  // - The remaining dynamic slices size.
  [[nodiscard]] auto Borrow(SizeType preferred_slice_capacity)
      -> BorrowResult override;
  auto Return(OwnedSlicePtr&& owned_ptr) -> ReturnResult override;

  [[nodiscard]] constexpr auto Size() const -> SizeType override;
  [[nodiscard]] constexpr auto Capacity() const -> SizeType override;
  [[nodiscard]] constexpr auto Empty() const -> bool override;
  [[nodiscard]] constexpr auto Full() const -> bool override;

 protected:
  auto Return(Slice* slice) -> ReturnResult override;

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
  // const auto buffer_size = std::min({
  //     preferred_slice_capacity,
  //     Capacity(),
  //     options_.max_slice_capacity,
  //     Capacity() - borrowed_items_size_,
  // });
  // if (buffer_size < 1) return Result::Err({});
  // borrowed_items_size_ += buffer_size;

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

  // if (preferred_slice_capacity < 1) return Result::Err({});
  // const auto capacity = Capacity();
  // const auto try_buffer_size = std::min(
  //     {preferred_slice_capacity, capacity, options_.max_slice_capacity});
  // const auto previous_size = borrowed_items_size_.fetch_add(try_buffer_size);
  // if (capacity < previous_size) return Result::Err({});
  // const auto new_size = previous_size + try_buffer_size;
  // if (new_size < 1) return Result::Err({});
  // if (capacity < new_size) {
  //   borrowed_items_size_.store(capacity);
  // }
  // const auto buffer_size = std::min(try_buffer_size, capacity -
  // previous_size);

  // auto* slice = allocator_.allocate(1);
  // allocator_.construct(slice, buffer_size);
  // auto* slice = std::allocator_traits<Allocator>::allocate(allocator_, 1);
  // std::allocator_traits<Allocator>::construct(allocator_, slice,
  // buffer_size); auto* buffer = static_cast<T*>(::operator new(sizeof(T)));
  // auto* slice = ::new(buffer) Slice{buffer_size};
  // auto* slice = new Slice{buffer_size};
  // return OwnedSlicePtr{slice, this};
}

template <class T>
auto DynamicBufferSlicePool<T>::Return(Slice* slice) -> ReturnResult {
  // allocator_.destroy(slice);
  // allocator_.deallocate(slice, 1);
  // std::allocator_traits<Allocator>::destroy(allocator_, slice);
  // std::allocator_traits<Allocator>::deallocate(allocator_, slice, 1);
  // slice->~Slice();
  // ::operator delete(slice);
  borrowed_items_size_ -= slice->Capacity();
  delete slice;
  return ReturnResult::Ok({});
}

template <class T>
auto DynamicBufferSlicePool<T>::Return(OwnedSlicePtr&& owned_ptr)
    -> ReturnResult {
  if (owned_ptr.Owner() != this) return PtrOwnerError::kDoesNotOwnPtr;
  return Return(owned_ptr.Take());
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

#endif
