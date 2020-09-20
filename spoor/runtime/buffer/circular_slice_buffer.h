#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_CIRCULAR_SLICE_BUFFER_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_CIRCULAR_SLICE_BUFFER_H_

#include <algorithm>
#include <functional>
#include <span>
#include <vector>

#include "spoor/runtime/buffer/amalgamated_buffer_slice_pool.h"
#include "spoor/runtime/buffer/buffer_slice.h"
#include "util/memory/owned_ptr.h"

namespace spoor::runtime::buffer {

template <class T>
class CircularSliceBuffer {
 public:
  using Slice = BufferSlice<T>;
  using OwnedSlicePtr = util::memory::OwnedPtr<Slice>;
  using SlicePool = AmalgamatedBufferSlicePool<T>;
  using SizeType = typename BufferSlicePool<T>::SizeType;
  using SlicesType = std::vector<OwnedSlicePtr>;
  using ValueType = T;

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

  auto Push(const T& item) -> void;
  auto Push(T&& item) -> void;
  auto Flush() -> void;
  auto Clear() -> void;
  [[nodiscard]] constexpr auto ContiguousMemoryChunks() -> std::vector<std::span<T>>;

  [[nodiscard]] constexpr auto Size() -> SizeType;
  [[nodiscard]] constexpr auto Capacity() -> SizeType;
  [[nodiscard]] constexpr auto Empty() -> bool;
  [[nodiscard]] constexpr auto Full() -> bool;

 private:
  Options options_;
  SlicesType slices_;
  SizeType size_;
  typename SlicesType::iterator insertion_iterator_;
};

template <class T>
CircularSliceBuffer<T>::CircularSliceBuffer(const Options& options)
    : options_{options},
      slices_{},
      size_{0},
      insertion_iterator_{slices_.begin()} {}

template <class T>
CircularSliceBuffer<T>::~CircularSliceBuffer() {
  options_.buffer_slice_pool->Return(std::move(slices_));
}

template <class T>
auto CircularSliceBuffer<T>::Push(const T& item) -> void {
  Push(std::move(item));  // TODO is this kosher?
}

template <class T>
auto CircularSliceBuffer<T>::Push(T&& item) -> void {
  if (insertion_iterator_->WillWrapOnNextPush()) {
    if (Full()) {
      if (options_.flush_when_full) Flush();
      insertion_iterator_ = slices_.begin();
    } else {
      if (std::next(insertion_iterator_) == slices_.end()) {
        auto result =
            options_.buffer_slice_pool->Borrow(options_.capacity - size_);
        if (result.IsOk()) {
          slices_.push_back(std::move(result.Ok().value()));
        } else {
          Flush();
          return;
        }
      }
      ++insertion_iterator_;
      ++size_;
    }
  }
  insertion_iterator_->Push(item);
}

template <class T>
auto CircularSliceBuffer<T>::Flush() -> void {
  if (!slices_.empty()) {
    options_.flush_handler(std::move(slices_));
  }
  Clear();
}

template <class T>
auto CircularSliceBuffer<T>::Clear() -> void {
  if (!slices_.empty()) {
    auto unowned_slices =
        options_.buffer_slice_pool->Return(std::move(slices_));
    assert(unowned_slices.size() == 0);
  }
  size_ = 0;
  insertion_iterator_ = slices_.begin();
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Size() -> SizeType {
  return size_;
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Capacity() -> SizeType {
  return options_.capacity;
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Empty() -> bool {
  return Size() == 0;
}

template <class T>
constexpr auto CircularSliceBuffer<T>::Full() -> bool {
  return Capacity() <= Size() && insertion_iterator_->Full();
}

template <class T>
constexpr auto CircularSliceBuffer<T>::ContiguousMemoryChunks()
    -> std::vector<std::span<T>> {
  if (Empty()) return {};
  std::vector<std::span<T>> chunks{};
  // Over allocating by one is preferable to performing several unnecessary heap
  // allocations and possible over allocating by more than one.
  chunks.reserve(2 * slices_.size());
  for (auto iterator = insertion_iterator_; iterator != slices_.end();
       ++iterator) {
    const auto slice_chunks = iterator->ContiguousMemoryChunks();
    chunks.insert(chunks.end(), slice_chunks.begin(), slice_chunks.end());
  }
  for (auto iterator = slices_.begin(); iterator != insertion_iterator_;
       ++iterator) {
    const auto slice_chunks = iterator->ContiguousMemoryChunks();
    chunks.insert(chunks.end(), slice_chunks.begin(), slice_chunks.end());
  }
  return chunks;
}

}  // namespace spoor::runtime::buffer

#endif
