#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_CONTIGUOUS_MEMORY_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_CONTIGUOUS_MEMORY_H_

#include <cstddef>

namespace spoor::runtime::buffer {

template <class T>
struct ContiguousMemory {
  T* begin;
  std::size_t size;
};

template <class T>
auto operator==(const ContiguousMemory<T>& lhs, const ContiguousMemory<T>& rhs)
    -> bool {
  return lhs.begin == rhs.begin && lhs.size == rhs.size;
}

}  // namespace spoor::runtime::buffer

#endif
