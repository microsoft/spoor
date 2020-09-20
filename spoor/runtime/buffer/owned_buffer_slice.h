// TODO decide if we need to call each items' destructor on clear and
// destruction
#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_OWNED_BUFFER_SLICE_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_OWNED_BUFFER_SLICE_H_

#include <span>
#include <vector>
#include <iostream>  // # TODO

#include "spoor/runtime/buffer/buffer_slice.h"

namespace spoor::runtime::buffer {

template <class T>
class OwnedBufferSlice final : public BufferSlice<T> {
 public:
  using SizeType = typename BufferSlice<T>::SizeType;

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
      -> std::vector<std::span<T>> override;

  [[nodiscard]] constexpr auto Size() const -> SizeType override;
  [[nodiscard]] constexpr auto Capacity() const -> SizeType override;
  [[nodiscard]] constexpr auto Empty() const -> bool override;
  [[nodiscard]] constexpr auto Full() const -> bool override;
  [[nodiscard]] constexpr auto WillWrapOnNextPush() const -> bool override;

 private:
  SizeType capacity_;  // TODO const?
  std::vector<T> buffer_;
  typename std::vector<T>::iterator insertion_iterator_;
};

template <class T>
constexpr OwnedBufferSlice<T>::OwnedBufferSlice(SizeType capacity)
    : capacity_{capacity}, buffer_{}, insertion_iterator_{buffer_.begin()} {
  buffer_.reserve(capacity);
}

template <class T>
constexpr auto OwnedBufferSlice<T>::Push(const T& item) -> void {
  if (Full()) {
    *insertion_iterator_ = item;
    ++insertion_iterator_;
  } else {
    buffer_.push_back(item);
    insertion_iterator_ = buffer_.end();
  }
  if (Full() && insertion_iterator_ == buffer_.end()) {
    insertion_iterator_ = buffer_.begin();
  }
}

template <class T>
constexpr auto OwnedBufferSlice<T>::Push(T&& item) -> void {
  if (Full()) {
    *insertion_iterator_ = std::move(item);
    ++insertion_iterator_;
  } else {
    buffer_.push_back(std::move(item));
    insertion_iterator_ = buffer_.end();
  }
  if (Full() && insertion_iterator_ == buffer_.end()) {
    insertion_iterator_ = buffer_.begin();
  }
}

template <class T>
constexpr auto OwnedBufferSlice<T>::Clear() -> void {
  buffer_.clear();
  insertion_iterator_ = buffer_.begin();
}

template <class T>
constexpr auto OwnedBufferSlice<T>::ContiguousMemoryChunks()
    -> std::vector<std::span<T>> {
  if (Empty()) return {};
  if (!Full() || insertion_iterator_ == buffer_.begin()) return {buffer_};
  const auto begin = buffer_.begin();
  const auto end = buffer_.end();
  const std::span<T> first_chunk{
      &(*insertion_iterator_),
      static_cast<SizeType>(std::distance(insertion_iterator_, end))};
  const std::span<T> second_chunk{
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
  if (Capacity() == 0) return true;
  if (Full()) return std::next(insertion_iterator_) == buffer_.end();
  return Capacity() <= Size() + 1;
}

}  // namespace spoor::runtime::buffer

#endif
