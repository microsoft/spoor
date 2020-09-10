#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_POOL_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_POOL_H_

#include <cstddef>

#include "spoor/runtime/buffer/buffer_slice.h"
#include "util/memory/owned_ptr.h"
#include "util/memory/ptr_owner.h"
#include "util/result.h"

namespace spoor::runtime::buffer {

template <class T>
class BufferSlicePool : public util::memory::PtrOwner<BufferSlice<T>> {
 public:
  enum class BorrowError;

  using Slice = BufferSlice<T>;
  using SizeType = typename Slice::SizeType;
  using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
  using BorrowResult = util::result::Result<OwnedSlicePtr, BorrowError>;

  enum class BorrowError {
    kNoSlicesAvailable,
    kCasAttemptsExhausted,
  };

  // TODO is it correct for all the constructors to be default?
  BufferSlicePool() = default;
  BufferSlicePool(const BufferSlicePool&) = default;
  BufferSlicePool(BufferSlicePool&&) noexcept = default;
  auto operator=(const BufferSlicePool&) -> BufferSlicePool& = default;
  auto operator=(BufferSlicePool&&) noexcept -> BufferSlicePool& = default;
  virtual ~BufferSlicePool() = default;

  [[nodiscard]] virtual auto Borrow(SizeType preferred_slice_capacity)
      -> BorrowResult = 0;

  [[nodiscard]] virtual constexpr auto Size() const -> SizeType = 0;
  [[nodiscard]] virtual constexpr auto Capacity() const -> SizeType = 0;
  [[nodiscard]] virtual constexpr auto Empty() const -> bool = 0;
  [[nodiscard]] virtual constexpr auto Full() const -> bool = 0;
};

}  // namespace spoor::runtime::buffer

#endif
