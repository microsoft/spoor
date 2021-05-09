// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "gsl/gsl"
#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "spoor/runtime/buffer/circular_buffer.h"
#include "util/memory/owned_ptr.h"

namespace spoor::runtime::buffer {

template <class T>
class CircularSliceBuffer;

template <class T>
constexpr auto operator==(const CircularSliceBuffer<T>& lhs,
                          const CircularSliceBuffer<T>& rhs) -> bool;

template <class T>
class CircularSliceBuffer final : public CircularBuffer<T> {
 public:
  using ValueType = T;
  using Slice = CircularBuffer<T>;
  using SizeType = typename CircularBuffer<T>::SizeType;
  using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
  using SlicePool = BufferSlicePool<T>;
  using SlicesType = std::vector<OwnedSlicePtr>;

  struct alignas(16) Options {
    gsl::not_null<SlicePool*> buffer_slice_pool;
    SizeType capacity;
  };

  CircularSliceBuffer() = delete;
  constexpr explicit CircularSliceBuffer(Options options);
  CircularSliceBuffer(const CircularSliceBuffer&) = delete;
  constexpr CircularSliceBuffer(CircularSliceBuffer&&) noexcept = default;
  auto operator=(const CircularSliceBuffer&) -> CircularSliceBuffer& = delete;
  constexpr auto operator=(CircularSliceBuffer&&) noexcept
      -> CircularSliceBuffer& = default;
  constexpr ~CircularSliceBuffer() override = default;

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
  friend auto operator==<>(const CircularSliceBuffer<T>& lhs,
                           const CircularSliceBuffer<T>& rhs) -> bool;

  Options options_;
  SlicesType slices_;
  SizeType size_;
  SizeType acquired_slices_capacity_;
  typename SlicesType::iterator insertion_iterator_;

  constexpr auto PrepareToPush() -> void;
};

template <class T>
constexpr CircularSliceBuffer<T>::CircularSliceBuffer(Options options)
    : options_{std::move(options)},
      slices_{},
      size_{0},
      acquired_slices_capacity_{0},
      insertion_iterator_{std::begin(slices_)} {}

template <class T>
constexpr auto CircularSliceBuffer<T>::Push(const T& item) -> void {
  if (Capacity() == 0) return;
  PrepareToPush();
  if (insertion_iterator_ == std::cend(slices_)) return;
  (*insertion_iterator_)->Push(item);
  size_ = std::min(size_ + 1, acquired_slices_capacity_);
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Push(T&& item) -> void {
  if (Capacity() == 0) return;
  PrepareToPush();
  if (insertion_iterator_ == std::cend(slices_)) return;
  (*insertion_iterator_)->Push(std::move(item));
  size_ = std::min(size_ + 1, acquired_slices_capacity_);
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Clear() -> void {
  for (auto& slice : slices_) {
    options_.buffer_slice_pool->Return(std::move(slice));
  }
  slices_.clear();
  size_ = 0;
  acquired_slices_capacity_ = 0;
  insertion_iterator_ = std::begin(slices_);
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Size() const -> SizeType {
  return size_;
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Capacity() const -> SizeType {
  return options_.capacity;
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Empty() const -> bool {
  return Size() == 0;
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Full() const -> bool {
  return Capacity() <= Size();
}

template <class T>
constexpr auto CircularSliceBuffer<T>::WillWrapOnNextPush() const -> bool {
  return (Capacity() == 0) ||
         (Capacity() <= acquired_slices_capacity_ &&
          insertion_iterator_ == std::prev(std::cend(slices_)) &&
          (*insertion_iterator_)->WillWrapOnNextPush());
}

template <class T>
constexpr auto CircularSliceBuffer<T>::ContiguousMemoryChunks()
    -> std::vector<gsl::span<T>> {
  if (Empty()) return {};
  std::vector<gsl::span<T>> chunks{};
  // Over allocating by one is preferable to performing several unnecessary heap
  // allocations and possibly over allocating by more than one.
  chunks.reserve(slices_.size() + 1);
  const auto insertion_iterator_chunks =
      (*insertion_iterator_)->ContiguousMemoryChunks();
  if (1 < insertion_iterator_chunks.size()) {
    chunks.emplace_back(insertion_iterator_chunks.front());
  }
  for (auto iterator = std::next(insertion_iterator_);
       iterator != std::cend(slices_); ++iterator) {
    const auto slice_chunks = (*iterator)->ContiguousMemoryChunks();
    chunks.insert(std::cend(chunks), std::cbegin(slice_chunks),
                  std::cend(slice_chunks));
  }
  for (auto iterator = std::cbegin(slices_); iterator != insertion_iterator_;
       ++iterator) {
    const auto slice_chunks = (*iterator)->ContiguousMemoryChunks();
    chunks.insert(std::cend(chunks), std::cbegin(slice_chunks),
                  std::cend(slice_chunks));
  }
  chunks.emplace_back(insertion_iterator_chunks.back());
  return chunks;
}

template <class T>
constexpr auto CircularSliceBuffer<T>::PrepareToPush() -> void {
  if (insertion_iterator_ != std::cend(slices_) &&
      (*insertion_iterator_)->WillWrapOnNextPush()) {
    ++insertion_iterator_;
  }
  if (insertion_iterator_ == std::cend(slices_)) {
    if (Capacity() <= acquired_slices_capacity_) {
      insertion_iterator_ = std::begin(slices_);
    } else {
      auto result = options_.buffer_slice_pool->Borrow(Capacity() - Size());
      if (result.IsOk()) {
        auto buffer_slice = std::move(result.Ok());
        acquired_slices_capacity_ += buffer_slice->Capacity();
        slices_.emplace_back(std::move(buffer_slice));
        insertion_iterator_ = std::prev(std::end(slices_));
      } else {
        insertion_iterator_ = std::begin(slices_);
      }
    }
  }
}

template <class T>
constexpr auto operator==(const CircularSliceBuffer<T>& lhs,
                          const CircularSliceBuffer<T>& rhs) -> bool {
  using SizeType = typename CircularSliceBuffer<T>::SizeType;
  if (lhs.Size() != rhs.Size()) return false;
  for (SizeType index{0}; index < lhs.slices_.size(); ++index) {
    const auto lhs_chunks = lhs.slices_.at(index)->ContiguousMemoryChunks();
    const auto rhs_chunks = rhs.slices_.at(index)->ContiguousMemoryChunks();
    if (lhs_chunks.size() != rhs_chunks.size()) return false;
    const auto equals =
        std::equal(std::cbegin(lhs_chunks), std::cend(lhs_chunks),
                   std::cbegin(rhs_chunks));
    if (!equals) return false;
  }
  return true;
}

}  // namespace spoor::runtime::buffer
