#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_POOL_OWNERSHIP_INFO_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_POOL_OWNERSHIP_INFO_H_

#include <atomic>
#include <optional>

namespace spoor::runtime::buffer {

template <class T>
class BufferSlicePool;

template <class T>
class BufferSlicePoolOwnershipInfo {
 public:
  BufferSlicePoolOwnershipInfo() = default;
  explicit BufferSlicePoolOwnershipInfo(std::atomic_size_t* pool_size);
  BufferSlicePoolOwnershipInfo(std::atomic_bool* reserved_pool_borrowed_flag,
                               std::atomic_size_t* pool_size);
 
 private:
  friend class BufferSlicePool<T>;

  std::optional<std::atomic_bool*> reserved_pool_borrowed_flag_{};
  std::atomic_size_t* pool_size_{};
};

template <class T>
BufferSlicePoolOwnershipInfo<T>::BufferSlicePoolOwnershipInfo(
    std::atomic_size_t* pool_size)
    : reserved_pool_borrowed_flag_{}, pool_size_{pool_size} {}

template <class T>
BufferSlicePoolOwnershipInfo<T>::BufferSlicePoolOwnershipInfo(
    std::atomic_bool* reserved_pool_borrowed_flag,
    std::atomic_size_t* pool_size)
    : reserved_pool_borrowed_flag_{reserved_pool_borrowed_flag},
      pool_size_{pool_size} {}

}  // namespace spoor::runtime::buffer

#endif
