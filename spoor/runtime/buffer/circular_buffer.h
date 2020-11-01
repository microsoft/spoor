#pragma once

#include <cstddef>
#include <vector>

#include "gsl/gsl"

namespace spoor::runtime::buffer {

template <class T>
class CircularBuffer {
 public:
  using ValueType = T;
  using SizeType = std::size_t;

  virtual constexpr ~CircularBuffer() = default;

  virtual constexpr auto Push(const T& item) -> void = 0;
  virtual constexpr auto Push(T&& item) -> void = 0;
  virtual constexpr auto Clear() -> void = 0;
  [[nodiscard]] virtual constexpr auto ContiguousMemoryChunks()
      -> std::vector<gsl::span<T>> = 0;

  [[nodiscard]] virtual constexpr auto Size() const -> SizeType = 0;
  [[nodiscard]] virtual constexpr auto Capacity() const -> SizeType = 0;
  [[nodiscard]] virtual constexpr auto Empty() const -> bool = 0;
  [[nodiscard]] virtual constexpr auto Full() const -> bool = 0;
  [[nodiscard]] virtual constexpr auto WillWrapOnNextPush() const -> bool = 0;

 protected:
  constexpr CircularBuffer() = default;
  constexpr CircularBuffer(const CircularBuffer&) = default;
  constexpr CircularBuffer(CircularBuffer&&) noexcept = default;
  constexpr auto operator=(const CircularBuffer&) -> CircularBuffer& = default;
  constexpr auto operator=(CircularBuffer&&) noexcept
      -> CircularBuffer& = default;
};

}  // namespace spoor::runtime::buffer
