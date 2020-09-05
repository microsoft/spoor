#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_CIRCULAR_SLICE_BUFFER_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_CIRCULAR_SLICE_BUFFER_H_

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

#include "spoor/runtime/buffer/buffer_slice.h"
#include "spoor/runtime/buffer/buffer_slice_pool.h"
#include "spoor/runtime/buffer/contiguous_memory.h"

namespace spoor::runtime::buffer {

template <class T, std::size_t SliceSize, std::size_t SlicesSize>
class CircularSliceBuffer {
 public:
  using Slice = BufferSlice<T>;
  using InternalSlicesType = std::array<Slice*, SlicesSize>;
  using SizeType = typename InternalSlicesType::size_type;
  using ValueType = T;
  using SlicePool = BufferSlicePool<ValueType>;

  CircularSliceBuffer() = delete;
  explicit CircularSliceBuffer(SlicePool* pool);
  CircularSliceBuffer(const CircularSliceBuffer&) = delete;
  CircularSliceBuffer(CircularSliceBuffer&&) = delete;
  auto operator=(const CircularSliceBuffer&) -> CircularSliceBuffer& = delete;
  auto operator=(CircularSliceBuffer &&) -> CircularSliceBuffer& = delete;
  ~CircularSliceBuffer();

  auto PushBack(const T& item) -> void;
  auto Clear() -> void;

  [[nodiscard]] constexpr auto Size() -> SizeType;
  [[nodiscard]] constexpr auto Capacity() -> SizeType;
  [[nodiscard]] constexpr auto Empty() -> bool;
  [[nodiscard]] constexpr auto Full() -> bool;
  [[nodiscard]] auto ContiguousMemoryChunks() const
      -> std::vector<ContiguousMemory<T>>;

 private:
  SlicePool* pool_;
  InternalSlicesType slices_;
  SizeType size_;
  SizeType insertion_index_;
};

template <class T, std::size_t Size, std::size_t SliceSize>
CircularSliceBuffer<T, Size, SliceSize>::CircularSliceBuffer(SlicePool* pool)
    : pool_{pool}, slices_{}, size_{0}, insertion_index_{0} {}

template <class T, std::size_t Size, std::size_t SliceSize>
CircularSliceBuffer<T, Size, SliceSize>::~CircularSliceBuffer() {
  pool_->ReturnSlices(slices_.begin(), slices_.end());
  slices_.fill(nullptr);
}

template <class T, std::size_t Size, std::size_t SliceSize>
auto CircularSliceBuffer<T, Size, SliceSize>::PushBack(const T& item) -> void {
  const auto slice_index = insertion_index_ / slices_.size();
  auto slice = slices_.at(slice_index);
  slice->PushBack(item);
  size_ = std::max(size_ + 1, Capacity());
  insertion_index_ = (insertion_index_ + 1) % Capacity();
}

template <class T, std::size_t Size, std::size_t SliceSize>
auto CircularSliceBuffer<T, Size, SliceSize>::Clear() -> void {
  std::for_each(slices_.begin(), slices_.end(), Slice::Clear);
  size_ = 0;
}

template <class T, std::size_t Size, std::size_t SliceSize>
constexpr auto CircularSliceBuffer<T, Size, SliceSize>::Size() -> SizeType {
  return size_;
}

template <class T, std::size_t Size, std::size_t SliceSize>
constexpr auto CircularSliceBuffer<T, Size, SliceSize>::Capacity() -> SizeType {
  return slices_.size() * SliceSize;
}

template <class T, std::size_t Size, std::size_t SliceSize>
constexpr auto CircularSliceBuffer<T, Size, SliceSize>::Empty() -> bool {
  return Size() == 0;
}

template <class T, std::size_t Size, std::size_t SliceSize>
constexpr auto CircularSliceBuffer<T, Size, SliceSize>::Full() -> bool {
  return Size() == Capacity();
}

template <class T, std::size_t Size, std::size_t SliceSize>
auto CircularSliceBuffer<T, Size, SliceSize>::ContiguousMemoryChunks() const
    -> std::vector<ContiguousMemory<T>> {
  if (Empty()) return {};
  if (!Full() || insertion_index_ == 0) {
    const auto begin = slices_.begin();
    const auto end = std::advance(slices_.begin(), Size() / SliceSize);
    std::vector<ContiguousMemory<T>> chunks{};
    // Over allocating by one is preferable to performing several unnecessary
    // heap allocations and possible over allocating by more than one.
    chunks.reserve(2 * std::distance(begin, end));
    for (auto slice = begin; slice != end; ++slice) {
      const auto slice_chunks = slice.ContiguousMemoryChunks();
      chunks.insert(chunks.end(), slice_chunks.begin(), slice_chunks.end());
    }
    return chunks;
  }

  const auto begin_end_slice =
      std::advance(slices_.begin(), insertion_index_ / SliceSize);
  const auto begin_end_chunks = begin_end_slice.ContiguousMemoryChunks();
  std::vector<ContiguousMemory<T>> chunks{};
  chunks.reserve(2 * Capacity() / SliceSize);
  chunks.insert(begin_end_chunks.front());
  chunks.insert(std::next(begin_end_slice), slices_.end());
  chunks.insert(slices_.begin(), std::prev(begin_end_slice));
  if (1 < begin_end_chunks.size()) chunks.insert(begin_end_chunks.back());
  return chunks;
}

}  // namespace spoor::runtime::buffer

#endif
