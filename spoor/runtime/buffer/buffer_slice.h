#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_H_

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <vector>
#include <span>

namespace spoor::runtime::buffer {

template <class T>
class BufferSlice {
  using ValueType = T;
  using SizeType = std::size_t;

  constexpr BufferSlice() = default;
  constexpr BufferSlice(const BufferSlice&) = default;
  constexpr BufferSlice(BufferSlice&&) noexcept = default;
  constexpr auto operator=(const BufferSlice&) = default;
  constexpr auto operator=(BufferSlice&&) noexcept = default;
  virtual ~BufferSlice() = default;

  virtual constexpr auto Push(const T& item) -> void = 0;
  virtual constexpr auto Push(T&& item) -> void = 0;
  virtual constexpr auto Clear() -> void = 0;

  [[nodiscard]] virtual constexpr auto Size() const -> SizeType = 0;
  [[nodiscard]] virtual constexpr auto Capacity() const -> SizeType = 0;
  [[nodiscard]] virtual constexpr auto Empty() const -> bool = 0;
  [[nodiscard]] virtual constexpr auto Full() const -> bool = 0;
  [[nodiscard]] virtual constexpr auto WillWrapOnNextPush() const -> bool = 0;
  [[nodiscard]] virtual constexpr auto ContiguousMemoryChunks() const
      -> std::vector<std::span<T>> = 0;
};

}  // namespace spoor::runtime::buffer

namespace spoor::runtime::buffer {

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

// template <class T>
// constexpr BufferSlice<T>::BufferSlice(const SizeType capacity)
//     : capacity_{capacity},
//       owns_buffer_{true},
//       buffer_{new ValueType[capacity]},
//       insertion_index_{0},
//       size_{0} {}
// 
// template <class T>
// constexpr BufferSlice<T>::BufferSlice(T* buffer, const SizeType capacity)
//     : capacity_{buffer == nullptr ? 0 : capacity},
//       owns_buffer_{false},
//       buffer_{buffer},
//       insertion_index_{0},
//       size_{0} {}
// 
// template <class T>
// constexpr BufferSlice<T>::BufferSlice(BufferSlice&& other) noexcept
//     : capacity_{std::move(other.capacity_)},
//       owns_buffer_{std::move(other.owns_buffer_)},
//       buffer_{other.buffer_},
//       insertion_index_{std::move(other.insertion_index_)},
//       size_{std::move(other.size_)} {
//   other.capacity_ = 0;
//   other.owns_buffer_ = false;
//   other.buffer_ = nullptr;
//   other.insertion_index_ = 0;
//   other.size_ = 0;
// }
// 
// // template <class T>
// // auto BufferSlice<T>::operator=(BufferSlice&& other) noexcept -> BufferSlice&
// // {
// //   if (this != &other) {
// //     delete[] buffer_;
// //     buffer_ = other.buffer_;
// //     other.buffer_ = nullptr;
// //
// //     options_ = std::move(other.options_);
// //     insertion_index_ = std::move(other.insertion_index_);
// //     size_ = std::move(other.size_);
// //   }
// //   return *this;
// // }
// 
// template <class T>
// BufferSlice<T>::~BufferSlice() {
//   if (owns_buffer_) {
//     delete[] buffer_;
//   } else {
//     for (SizeType offset{0}; offset < size_; ++offset) {
//       auto* item = std::next(buffer_, offset);
//       item->~T();
//     }
//   }
// }
// 
// template <class T>
// constexpr auto BufferSlice<T>::Push(const T& item) -> void {
//   if (buffer_ == nullptr) return;
//   buffer_[insertion_index_] = item;
//   insertion_index_ = (insertion_index_ + 1) % Capacity();
//   size_ = std::min(size_ + 1, Capacity());
// }
// 
// template <class T>
// constexpr auto BufferSlice<T>::Clear() -> void {
//   insertion_index_ = 0;
//   size_ = 0;
// }
// 
// template <class T>
// constexpr auto BufferSlice<T>::Size() const -> SizeType {
//   return size_;
// }
// 
// template <class T>
// constexpr auto BufferSlice<T>::Capacity() const -> SizeType {
//   return capacity_;
// }
// 
// template <class T>
// constexpr auto BufferSlice<T>::Empty() const -> bool {
//   return Size() == 0;
// }
// 
// template <class T>
// constexpr auto BufferSlice<T>::Full() const -> bool {
//   return Size() == Capacity();
// }
// 
// template <class T>
// constexpr auto BufferSlice<T>::WillWrapOnNextPush() const -> bool {
//   return (Capacity() == 0) || (insertion_index_ + 1) == Capacity();
// }
// 
// template <class T>
// constexpr auto BufferSlice<T>::ContiguousMemoryChunks() const
//     -> std::vector<ContiguousMemory<T>> {
//   if (Empty()) return {};
//   const auto value_type_size = sizeof(T);
//   if (insertion_index_ == 0 || insertion_index_ == Size()) {
//     return {{buffer_, Size() * value_type_size}};
//   }
//   ContiguousMemory<T> first_chunk{
//       buffer_ + insertion_index_,
//       (Capacity() - insertion_index_) * value_type_size};
//   ContiguousMemory<T> second_chunk{buffer_, (insertion_index_)*value_type_size};
//   return {first_chunk, second_chunk};
// }

}  // namespace spoor::runtime::buffer

#endif
