#pragma once

#include <iterator>
#include <utility>
#include <vector>

#include "gsl/gsl"
#include "spoor/runtime/buffer/circular_buffer.h"

namespace spoor::runtime::buffer {

template <class T>
class OwnedBufferSlice final : public CircularBuffer<T> {
 public:
  using SizeType = typename CircularBuffer<T>::SizeType;

  OwnedBufferSlice() = delete;
  constexpr explicit OwnedBufferSlice(SizeType capacity);
  OwnedBufferSlice(const OwnedBufferSlice&) = delete;
  constexpr OwnedBufferSlice(OwnedBufferSlice&& other) noexcept = default;
  auto operator=(const OwnedBufferSlice&) -> OwnedBufferSlice& = delete;
  auto operator=(OwnedBufferSlice&&) noexcept -> OwnedBufferSlice& = delete;
  constexpr ~OwnedBufferSlice() = default;

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
  SizeType capacity_;
  std::vector<T> buffer_;
  typename std::vector<T>::iterator insertion_iterator_;
};

}  // namespace spoor::runtime::buffer
