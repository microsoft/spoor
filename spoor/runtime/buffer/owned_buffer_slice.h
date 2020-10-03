#pragma once

#include <vector>

#include "gsl/gsl"
#include "spoor/runtime/buffer/circular_buffer.h"

namespace spoor::runtime::buffer {

// TODO decide if we need to call each items' destructor on clear and
// destruction
template <class T>
class OwnedBufferSlice final : public CircularBuffer<T> {
 public:
  using SizeType = typename CircularBuffer<T>::SizeType;

  OwnedBufferSlice() = delete;
  explicit constexpr OwnedBufferSlice(SizeType capacity);
  OwnedBufferSlice(const OwnedBufferSlice&) = delete;
  // TODO is the default sufficient?
  constexpr OwnedBufferSlice(OwnedBufferSlice&& other) noexcept = default;
  auto operator=(const OwnedBufferSlice&) -> OwnedBufferSlice& = delete;
  auto operator=(OwnedBufferSlice&&) noexcept -> OwnedBufferSlice& = delete;
  ~OwnedBufferSlice() = default;

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

template <class T>
constexpr OwnedBufferSlice<T>::OwnedBufferSlice(SizeType capacity)
    : capacity_{capacity}, buffer_{}, insertion_iterator_{std::begin(buffer_)} {
  buffer_.reserve(capacity);
}

template <class T>
constexpr auto OwnedBufferSlice<T>::Push(const T& item) -> void {
  if (Capacity() == 0) return;
  if (Full() && insertion_iterator_ == std::end(buffer_)) {
    insertion_iterator_ = std::begin(buffer_);
  }
  if (Full()) {
    *insertion_iterator_ = item;
    ++insertion_iterator_;
  } else {
    buffer_.push_back(item);
    insertion_iterator_ = buffer_.end();
  }
}

template <class T>
constexpr auto OwnedBufferSlice<T>::Push(T&& item) -> void {
  if (Capacity() == 0) return;
  if (Full() && insertion_iterator_ == std::end(buffer_)) {
    insertion_iterator_ = std::begin(buffer_);
  }
  if (Full()) {
    *insertion_iterator_ = std::move(item);
    ++insertion_iterator_;
  } else {
    buffer_.push_back(std::move(item));
    insertion_iterator_ = buffer_.end();
  }
}

template <class T>
constexpr auto OwnedBufferSlice<T>::Clear() -> void {
  buffer_.clear();
  insertion_iterator_ = std::begin(buffer_);
}

template <class T>
constexpr auto OwnedBufferSlice<T>::ContiguousMemoryChunks()
    -> std::vector<gsl::span<T>> {
  if (Empty()) return {};
  if (!Full() || insertion_iterator_ == std::end(buffer_)) return {buffer_};
  const auto begin = std::begin(buffer_);
  const auto end = std::end(buffer_);
  const gsl::span<T> first_chunk{
      &(*insertion_iterator_),
      static_cast<SizeType>(std::distance(insertion_iterator_, end))};
  const gsl::span<T> second_chunk{
      &(*begin),
      static_cast<SizeType>(std::distance(begin, insertion_iterator_))};
  return {first_chunk, second_chunk};
}

template <class T>
constexpr auto OwnedBufferSlice<T>::Size() const -> SizeType {
  return buffer_.size();
}

template <class T>
constexpr auto OwnedBufferSlice<T>::Capacity() const -> SizeType {
  return capacity_;
}

template <class T>
constexpr auto OwnedBufferSlice<T>::Empty() const -> bool {
  return buffer_.empty();
}

template <class T>
constexpr auto OwnedBufferSlice<T>::Full() const -> bool {
  return Capacity() <= Size();
}

template <class T>
constexpr auto OwnedBufferSlice<T>::WillWrapOnNextPush() const -> bool {
  if (!Full()) return false;
  return insertion_iterator_ == std::end(buffer_);
}

}  // namespace spoor::runtime::buffer
