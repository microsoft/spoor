#pragma once

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "gsl/gsl"
#include "spoor/runtime/buffer/circular_buffer.h"

namespace spoor::runtime::buffer {

template <class T>
class UnownedBufferSlice final : public CircularBuffer<T> {
 public:
  using SizeType = typename CircularBuffer<T>::SizeType;

  UnownedBufferSlice() = delete;
  constexpr explicit UnownedBufferSlice(gsl::span<T> buffer);
  UnownedBufferSlice(const UnownedBufferSlice&) = delete;
  constexpr UnownedBufferSlice(UnownedBufferSlice&& other) noexcept = default;
  auto operator=(const UnownedBufferSlice&) -> UnownedBufferSlice& = delete;
  auto operator=(UnownedBufferSlice&&) noexcept -> UnownedBufferSlice& = delete;
  constexpr ~UnownedBufferSlice() = default;

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
  gsl::span<T> buffer_;
  typename gsl::span<T>::iterator insertion_iterator_;
  SizeType size_;
};

}  // namespace spoor::runtime::buffer
