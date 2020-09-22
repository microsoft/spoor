#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_H_

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <span>
#include <vector>

namespace spoor::runtime::buffer {

template <class T>
class CircularBuffer {
 public:
  using ValueType = T;
  using SizeType = std::size_t;

  constexpr CircularBuffer() = default;
  constexpr CircularBuffer(const CircularBuffer&) = default;
  constexpr CircularBuffer(CircularBuffer&&) noexcept = default;
  constexpr auto operator=(const CircularBuffer&) -> CircularBuffer& = default;
  constexpr auto operator=(CircularBuffer&&) noexcept
      -> CircularBuffer& = default;
  virtual ~CircularBuffer() = default;

  virtual constexpr auto Push(const T& item) -> void = 0;
  virtual constexpr auto Push(T&& item) -> void = 0;
  virtual constexpr auto Clear() -> void = 0;
  [[nodiscard]] virtual constexpr auto ContiguousMemoryChunks()
      -> std::vector<std::span<T>> = 0;

  [[nodiscard]] virtual constexpr auto Size() const -> SizeType = 0;
  [[nodiscard]] virtual constexpr auto Capacity() const -> SizeType = 0;
  [[nodiscard]] virtual constexpr auto Empty() const -> bool = 0;
  [[nodiscard]] virtual constexpr auto Full() const -> bool = 0;
  [[nodiscard]] virtual constexpr auto WillWrapOnNextPush() const -> bool = 0;
};

}  // namespace spoor::runtime::buffer

#endif
