#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_CIRCULAR_SLICE_BUFFER_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_CIRCULAR_SLICE_BUFFER_H_

#include <algorithm>
#include <functional>
#include <span>
#include <vector>
#include <iostream> // TODO

#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "spoor/runtime/buffer/circular_buffer.h"
#include "util/memory/owned_ptr.h"

namespace spoor::runtime::buffer {

template <class T>
class CircularSliceBuffer final : public CircularBuffer<T> {
 public:
  using ValueType = T;
  using Slice = CircularBuffer<T>;
  using SizeType = typename CircularBuffer<T>::SizeType;
  using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
  using SlicePool = BufferSlicePool<T>;
  using SlicesType = std::vector<OwnedSlicePtr>;

  struct Options {
    std::function<void(SlicesType&&)> flush_handler;
    SlicePool* buffer_slice_pool;
    SizeType capacity;
    bool flush_when_full;
  };

  CircularSliceBuffer() = delete;
  explicit CircularSliceBuffer(const Options& options);
  CircularSliceBuffer(const CircularSliceBuffer&) = delete;
  CircularSliceBuffer(CircularSliceBuffer&&) noexcept;
  auto operator=(const CircularSliceBuffer&) -> CircularSliceBuffer& = delete;
  auto operator=(CircularSliceBuffer&&) noexcept -> CircularSliceBuffer&;
  ~CircularSliceBuffer();

  constexpr auto Push(const T& item) -> void override;
  constexpr auto Push(T&& item) -> void override;
  constexpr auto Clear() -> void override;
  [[nodiscard]] constexpr auto ContiguousMemoryChunks()
      -> std::vector<std::span<T>> override;

  constexpr auto Flush() -> void;

  [[nodiscard]] constexpr auto Size() const -> SizeType override;
  [[nodiscard]] constexpr auto Capacity() const -> SizeType override;
  [[nodiscard]] constexpr auto Empty() const -> bool override;
  [[nodiscard]] constexpr auto Full() const -> bool override;
  [[nodiscard]] constexpr auto WillWrapOnNextPush() const -> bool override;

 private:
  Options options_;
  SlicesType slices_;
  SizeType size_;
  typename SlicesType::iterator insertion_iterator_;

  auto PrepareToPush() -> void; // TODO constexpr?
};

template <class T>
CircularSliceBuffer<T>::CircularSliceBuffer(const Options& options)
    : options_{options},
      slices_{},
      size_{0},
      insertion_iterator_{slices_.begin()} {}

template <class T>
CircularSliceBuffer<T>::~CircularSliceBuffer() {
  // options_.buffer_slice_pool->Return(std::move(slices_));
  // TODO
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Push(const T& item) -> void {
  PrepareToPush();
  (*insertion_iterator_)->Push(item);
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Push(T&& item) -> void {
  PrepareToPush();
  (*insertion_iterator_)->Push(std::move(item));
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Flush() -> void {
  if (!slices_.empty()) {
    options_.flush_handler(std::move(slices_));
  }
  Clear();
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Clear() -> void {
  if (!slices_.empty()) {
    for (auto& slice : slices_) {
      options_.buffer_slice_pool->Return(std::move(slice));
      // TODO result?
    }
    // auto unowned_slices =
    //     options_.buffer_slice_pool->Return(std::move(slices_));
    // assert(unowned_slices.size() == 0);
  }
  size_ = 0;
  insertion_iterator_ = slices_.begin();
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
  return Capacity() <= Size() && (*insertion_iterator_)->Full();
}

template <class T>
constexpr auto CircularSliceBuffer<T>::WillWrapOnNextPush() const -> bool {
  return false; // TODO
}

template <class T>
constexpr auto CircularSliceBuffer<T>::ContiguousMemoryChunks()
    -> std::vector<std::span<T>> {
  if (Empty()) return {};
  std::vector<std::span<T>> chunks{};
  // Over allocating by one is preferable to performing several unnecessary heap
  // allocations and possible over allocating by more than one.
  // chunks.reserve(2 * slices_.size()); // TODO
  for (auto iterator = insertion_iterator_; iterator != slices_.end();
       ++iterator) {
    const auto slice_chunks = (*iterator)->ContiguousMemoryChunks();
    chunks.insert(chunks.end(), slice_chunks.begin(), slice_chunks.end());
  }
  for (auto iterator = slices_.begin(); iterator != insertion_iterator_;
       ++iterator) {
    const auto slice_chunks = (*iterator)->ContiguousMemoryChunks();
    chunks.insert(chunks.end(), slice_chunks.cbegin(), slice_chunks.cend());
  }
  return chunks;
}

template <class T>
auto CircularSliceBuffer<T>::PrepareToPush() -> void {
  if (insertion_iterator_ != slices_.end() && (*insertion_iterator_)->WillWrapOnNextPush()) {
    if (Full()) {
      if (options_.flush_when_full) Flush();
      insertion_iterator_ = slices_.begin();
    } else {
      if (std::next(insertion_iterator_) == slices_.end()) {
        auto result = options_.buffer_slice_pool->Borrow(Capacity() - Size());
        if (result.IsOk()) {
          slices_.push_back(std::move(result.Ok()));
        } else {
          Flush();
          return;
        }
      }
      ++insertion_iterator_;
      ++size_;
    }
  }
}

}  // namespace spoor::runtime::buffer

#endif
