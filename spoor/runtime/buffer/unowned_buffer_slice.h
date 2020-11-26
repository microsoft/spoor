// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

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

template <class T>
constexpr UnownedBufferSlice<T>::UnownedBufferSlice(gsl::span<T> buffer)
    : buffer_{buffer}, insertion_iterator_{std::begin(buffer_)}, size_{0} {}

template <class T>
constexpr auto UnownedBufferSlice<T>::Push(const T& item) -> void {
  if (Capacity() == 0) return;
  if (WillWrapOnNextPush()) insertion_iterator_ = std::begin(buffer_);
  *insertion_iterator_ = item;
  size_ = std::min(size_ + 1, Capacity());
  ++insertion_iterator_;
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Push(T&& item) -> void {
  if (Capacity() == 0) return;
  if (WillWrapOnNextPush()) insertion_iterator_ = std::begin(buffer_);
  *insertion_iterator_ = std::move(item);
  size_ = std::min(size_ + 1, Capacity());
  ++insertion_iterator_;
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Clear() -> void {
  insertion_iterator_ = std::begin(buffer_);
  size_ = 0;
}

template <class T>
constexpr auto UnownedBufferSlice<T>::ContiguousMemoryChunks()
    -> std::vector<gsl::span<T>> {
  if (Empty()) return {};
  const auto begin = std::cbegin(buffer_);
  const auto end = std::cend(buffer_);
  if (!Full() || insertion_iterator_ == end) return {{&(*begin), Size()}};
  const gsl::span<T> first_chunk{
      &(*insertion_iterator_),
      static_cast<SizeType>(std::distance(insertion_iterator_, end))};
  const gsl::span<T> second_chunk{
      &(*begin),
      static_cast<SizeType>(std::distance(begin, insertion_iterator_))};
  return {first_chunk, second_chunk};
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Size() const -> SizeType {
  return size_;
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Capacity() const -> SizeType {
  return buffer_.size();
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Empty() const -> bool {
  return Size() == 0;
}

template <class T>
constexpr auto UnownedBufferSlice<T>::Full() const -> bool {
  return Capacity() <= Size();
}

template <class T>
constexpr auto UnownedBufferSlice<T>::WillWrapOnNextPush() const -> bool {
  return insertion_iterator_ == std::cend(buffer_);
}

}  // namespace spoor::runtime::buffer
