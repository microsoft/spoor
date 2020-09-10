#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_H_

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <vector>

#include "spoor/runtime/buffer/contiguous_memory.h"

namespace spoor::runtime::buffer {

template <class T>
class ReservedBufferSlicePool;

template <class T>
class DynamicBufferSlicePool;

template <class T>
class BufferSlice {
 public:
  using SizeType = std::size_t;
  using ValueType = T;

  BufferSlice() = delete;
  explicit constexpr BufferSlice(SizeType capacity);
  constexpr BufferSlice(T* buffer, SizeType capacity);
  BufferSlice(const BufferSlice&) = delete;
  constexpr BufferSlice(BufferSlice&& other) noexcept;
  ~BufferSlice();
  auto operator=(const BufferSlice&) -> BufferSlice& = delete;
  auto operator=(BufferSlice&&) noexcept -> BufferSlice& = delete;

  constexpr auto Push(const T& item) -> void;
  // auto Push(T&& item) -> void;  // TODO
  constexpr auto Clear() -> void;

  [[nodiscard]] constexpr auto Size() const -> SizeType;
  [[nodiscard]] constexpr auto Capacity() const -> SizeType;
  [[nodiscard]] constexpr auto Empty() const -> bool;
  [[nodiscard]] constexpr auto Full() const -> bool;
  [[nodiscard]] constexpr auto WillWrapOnNextPush() const -> bool;
  [[nodiscard]] constexpr auto ContiguousMemoryChunks() const
      -> std::vector<ContiguousMemory<T>>;

 private:
  // friend class ReservedBufferSlicePool<T>;
  // friend class DynamicBufferSlicePool<T>;
  // friend class BufferSlicePoolAllocator<T>;

  // using Allocator = BufferSlicePoolAllocator<T>;
  // friend auto std::allocator_traits<Allocator>::allocate(Allocator&,
  //                                                        std::size_t) -> T*;
  // friend auto std::allocator_traits<Allocator>::deallocate(Allocator&, T*,
  //                                                          std::size_t)
  //                                                          noexcept
  //     -> void;
  // template <class... Args>
  // friend auto std::allocator_traits<Allocator>::construct(Allocator&, T*,
  //                                                         Args...) -> T*;
  // friend auto std::allocator_traits<Allocator>::destroy(Allocator&, T*) ->
  // void;

  // explicit BufferSlice(SizeType capacity);
  // BufferSlice(SizeType capacity, T* buffer_);
  // ~BufferSlice();

  SizeType capacity_;
  bool owns_buffer_;
  ValueType* buffer_;
  SizeType insertion_index_;
  SizeType size_;
};

// static_assert(
//     !std::is_constructible_v<BufferSlice<char>>,
//     "`BufferSlice` should only be constructible from inside its object
//     pool.");

// template <class T>
// BufferSlice<T>::BufferSlice()
//     : capacity_{0},
//       owns_buffer_{true},
//       buffer_{nullptr},
//       insertion_index_{0},
//       size_{0} {}

template <class T>
constexpr BufferSlice<T>::BufferSlice(const SizeType capacity)
    : capacity_{capacity},
      owns_buffer_{true},
      buffer_{new ValueType[capacity]},
      insertion_index_{0},
      size_{0} {}

template <class T>
constexpr BufferSlice<T>::BufferSlice(T* buffer, const SizeType capacity)
    : capacity_{buffer == nullptr ? 0 : capacity},
      owns_buffer_{false},
      buffer_{buffer},
      insertion_index_{0},
      size_{0} {}

template <class T>
constexpr BufferSlice<T>::BufferSlice(BufferSlice&& other) noexcept
    : capacity_{std::move(other.capacity_)},
      owns_buffer_{std::move(other.owns_buffer_)},
      buffer_{other.buffer_},
      insertion_index_{std::move(other.insertion_index_)},
      size_{std::move(other.size_)} {
  other.capacity_ = 0;
  other.owns_buffer_ = false;
  other.buffer_ = nullptr;
  other.insertion_index_ = 0;
  other.size_ = 0;
}

// template <class T>
// auto BufferSlice<T>::operator=(BufferSlice&& other) noexcept -> BufferSlice&
// {
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
  } else {
    for (SizeType offset{0}; offset < size_; ++offset) {
      auto* item = std::next(buffer_, offset);
      item->~T();
    }
  }
}

template <class T>
constexpr auto BufferSlice<T>::Push(const T& item) -> void {
  if (buffer_ == nullptr) return;
  buffer_[insertion_index_] = item;
  insertion_index_ = (insertion_index_ + 1) % Capacity();
  size_ = std::min(size_ + 1, Capacity());
}

template <class T>
constexpr auto BufferSlice<T>::Clear() -> void {
  insertion_index_ = 0;
  size_ = 0;
}

template <class T>
constexpr auto BufferSlice<T>::Size() const -> SizeType {
  return size_;
}

template <class T>
constexpr auto BufferSlice<T>::Capacity() const -> SizeType {
  return capacity_;
}

template <class T>
constexpr auto BufferSlice<T>::Empty() const -> bool {
  return Size() == 0;
}

template <class T>
constexpr auto BufferSlice<T>::Full() const -> bool {
  return Size() == Capacity();
}

template <class T>
constexpr auto BufferSlice<T>::WillWrapOnNextPush() const -> bool {
  return (Capacity() == 0) || (insertion_index_ + 1) == Capacity();
}

template <class T>
constexpr auto BufferSlice<T>::ContiguousMemoryChunks() const
    -> std::vector<ContiguousMemory<T>> {
  if (Empty()) return {};
  const auto value_type_size = sizeof(T);
  if (insertion_index_ == 0 || insertion_index_ == Size()) {
    return {{buffer_, Size() * value_type_size}};
  }
  ContiguousMemory<T> first_chunk{
      buffer_ + insertion_index_,
      (Capacity() - insertion_index_) * value_type_size};
  ContiguousMemory<T> second_chunk{buffer_, (insertion_index_)*value_type_size};
  return {first_chunk, second_chunk};
}

}  // namespace spoor::runtime::buffer

#endif
