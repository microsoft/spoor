#pragma once

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "gsl/gsl"
#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "spoor/runtime/buffer/circular_buffer.h"
#include "util/memory/owned_ptr.h"

namespace spoor::runtime::buffer {

template <class T>
class CircularSliceBuffer;

template <class T>
constexpr auto operator==(const CircularSliceBuffer<T>& lhs,
                          const CircularSliceBuffer<T>& rhs) -> bool;

template <class T>
class CircularSliceBuffer final : public CircularBuffer<T> {
 public:
  using ValueType = T;
  using Slice = CircularBuffer<T>;
  using SizeType = typename CircularBuffer<T>::SizeType;
  using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
  using SlicePool = BufferSlicePool<T>;
  using SlicesType = std::vector<OwnedSlicePtr>;

  struct Options {
    SlicePool* buffer_slice_pool;
    SizeType capacity;
  };

  CircularSliceBuffer() = delete;
  constexpr explicit CircularSliceBuffer(const Options& options);
  CircularSliceBuffer(const CircularSliceBuffer&) = delete;
  constexpr CircularSliceBuffer(CircularSliceBuffer&&) noexcept = default;
  auto operator=(const CircularSliceBuffer&) -> CircularSliceBuffer& = delete;
  constexpr auto operator=(CircularSliceBuffer&&) noexcept
      -> CircularSliceBuffer& = default;
  constexpr ~CircularSliceBuffer() = default;

  constexpr auto Push(const T& item) -> void override;
  constexpr auto Push(T&& item) -> void override;
  constexpr auto Clear() -> void override;
  [[nodiscard]] constexpr auto ContiguousMemoryChunks()
      -> std::vector<gsl::span<T>> override;

  [[nodiscard]] constexpr auto Size() const -> SizeType override;
  [[nodiscard]] constexpr auto Capacity() const -> SizeType override;
  [[nodiscard]] constexpr auto Empty() const -> bool override;
  [[nodiscard]] constexpr auto Full() const -> bool override;
  [[nodiscard]] constexpr auto WillWrapOnNextPush() const -> bool override;

 private:
  friend auto operator==<>(const CircularSliceBuffer<T>& lhs,
                           const CircularSliceBuffer<T>& rhs) -> bool;

  Options options_;
  SlicesType slices_;
  SizeType size_;
  SizeType acquired_slices_capacity_;
  typename SlicesType::iterator insertion_iterator_;

  constexpr auto PrepareToPush() -> void;
};

}  // namespace spoor::runtime::buffer
