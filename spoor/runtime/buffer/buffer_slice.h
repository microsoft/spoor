#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_H_

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <vector>
#include <memory>

#include "spoor/runtime/buffer/contiguous_memory.h"

namespace spoor::runtime::buffer {

template <class T>
class BufferSlicePool;

template <class T>
class BufferSlice {
 public:
  using SizeType = std::size_t;
  using ValueType = T;

  struct Options {
    SizeType capacity;
  };

  BufferSlice() = delete;
  BufferSlice(const BufferSlice&) = delete;
  BufferSlice(BufferSlice&&) noexcept = delete;
  auto operator=(const BufferSlice&) -> BufferSlice& = delete;
  auto operator=(BufferSlice&& other) noexcept -> BufferSlice& = delete;

  auto Push(const T& item) -> void;
  auto Clear() -> void;

  [[nodiscard]] constexpr auto Size() -> SizeType;
  [[nodiscard]] constexpr auto Capacity() -> SizeType;
  [[nodiscard]] constexpr auto Empty() -> bool;
  [[nodiscard]] auto ContiguousMemoryChunks()
      -> std::vector<ContiguousMemory<T>>;

 private:
  friend class BufferSlicePool<T>;

  explicit BufferSlice(const Options& options);
  BufferSlice(const Options& options, T* buffer_);
  ~BufferSlice();

  Options options_;
  bool owns_buffer_;
  ValueType* buffer_;
  SizeType insertion_index_;
  SizeType size_;
};

static_assert(
    !std::is_constructible_v<BufferSlice<char>>,
    "`BufferSlice` should only be constructible from inside its object pool.");

template <class T>
BufferSlice<T>::BufferSlice(const Options& options)
    : options_{options},
      owns_buffer_{true},
      buffer_{new ValueType[options.capacity]},
      insertion_index_{0},
      size_{0} {}

template <class T>
BufferSlice<T>::BufferSlice(const Options& options, T* buffer)
    : options_{options},
      owns_buffer_{false},
      buffer_{buffer},
      insertion_index_{0},
      size_{0} {}

// template <class T>
// auto BufferSlice<T>::operator=(BufferSlice&& other) noexcept -> BufferSlice& {
//   if (this != &other) {
//     delete[] buffer_;
//     buffer_ = other.buffer_;
//     other.buffer_ = nullptr;
// 
//     options_ = std::move(other.options_);
//     insertion_index_ = std::move(other.insertion_index_);
//     size_ = std::move(other.size_);
//   }
//   return *this;
// }

template <class T>
BufferSlice<T>::~BufferSlice() {
  if (owns_buffer_) {
    delete[] buffer_;
  } else  {
    for (SizeType i{0}; i < size_; ++i) {
      auto* item = std::next(buffer_, i);
      item->~T();
    }
  }
}

template <class T>
auto BufferSlice<T>::Push(const T& item) -> void {
  buffer_[insertion_index_] = item;
  insertion_index_ = (insertion_index_ + 1) % Capacity();
  size_ = std::min(size_ + 1, Capacity());

  // if (buffer_.size() < buffer_.capacity()) {
  //   buffer_.push_back(item);
  // } else {
  //   *insertion_iterator_ = item;
  // }
  // ++insertion_iterator_;
  // if (insertion_iterator_ == buffer_.end() &&
  //     buffer_.size() == buffer_.capacity()) {
  //   insertion_iterator_ = buffer_.begin();
  //}
}

template <class T>
auto BufferSlice<T>::Clear() -> void {
  insertion_index_ = 0;
  size_ = 0;

  // buffer_.clear();
  // insertion_iterator_ = buffer_.begin();
}

template <class T>
constexpr auto BufferSlice<T>::Size() -> SizeType {
  return size_;
  // return buffer_.size();
}

template <class T>
constexpr auto BufferSlice<T>::Capacity() -> SizeType {
  return options_.capacity;
  // return buffer_.capacity();
}

template <class T>
constexpr auto BufferSlice<T>::Empty() -> bool {
  return Size() == 0;
  // return buffer_.empty();
}

template <class T>
auto BufferSlice<T>::ContiguousMemoryChunks()
    -> std::vector<ContiguousMemory<T>> {
  if (Empty()) return {};
  const auto value_type_size = sizeof(ValueType);
  if (insertion_index_ == 0 || insertion_index_ == Size()) {
    return {{buffer_, Size() * value_type_size}};
  }
  ContiguousMemory<T> first_chunk{
      buffer_ + insertion_index_,
      (Capacity() - insertion_index_) * value_type_size};
  ContiguousMemory<T> second_chunk{buffer_, (insertion_index_)*value_type_size};
  return {first_chunk, second_chunk};
  // if (insertion_iterator_ == buffer_.begin() ||
  //     insertion_iterator_ == buffer_.end()) {
  //   return {{&(*(buffer_.begin())), buffer_.size() * type_size}};
  // }
  // ContiguousMemory<T> first_chunk{
  //     &(*insertion_iterator_),
  //     std::distance(insertion_iterator_, buffer_.end()) * type_size};
  // ContiguousMemory<T> second_chunk{
  //     &(*(buffer_.begin())),
  //     std::distance(buffer_.begin(), insertion_iterator_) * type_size};
  // return {first_chunk, second_chunk};
}

}  // namespace spoor::runtime::buffer

#endif
