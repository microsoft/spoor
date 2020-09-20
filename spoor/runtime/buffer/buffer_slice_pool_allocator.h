#ifndef SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_POOL_ALLOCATOR_H_
#define SPOOR_SPOOR_RUNTIME_BUFFER_BUFFER_SLICE_POOL_ALLOCATOR_H_

#include <memory>
#include <new>

namespace spoor::runtime::buffer {

template <class T>
class BufferSlicePoolAllocator : std::allocator<T> {
 public:
  using value_type = T;  // NOLINT(readability-identifier-naming)

  // NOLINTNEXTLINE(readability-identifier-naming)
  auto allocate(std::size_t size) -> T*;
  // NOLINTNEXTLINE(readability-identifier-naming)
  auto deallocate(T* ptr, std::size_t) noexcept -> void;
};

template <class T>  // NOLINTNEXTLINE(readability-identifier-naming)
auto BufferSlicePoolAllocator<T>::allocate(const std::size_t size) -> T* {
  return static_cast<T*>(::operator new(size * sizeof(T)));
}

template <class T>  // NOLINTNEXTLINE(readability-identifier-naming)
auto BufferSlicePoolAllocator<T>::deallocate(T* ptr, const std::size_t) noexcept
    -> void {
  ::operator delete(ptr);
}

}  // namespace spoor::runtime::buffer

// namespace std {
//
// template <class T>
// struct allocator_traits<spoor::runtime::buffer::BufferSlicePoolAllocator<T>>
// {
//   using Allocator = spoor::runtime::buffer::BufferSlicePoolAllocator<T>;
//   using size_type = size_t;
//   using difference_type = ptrdiff_t;
//   using pointer = T*;
//   using const_pointer = const T*;
//
//   [[nodiscard]] static constexpr auto allocate(Allocator& allocator, size_t
//   size) -> T*; static constexpr auto deallocate(Allocator& a, T* p, size_t n)
//   -> void; template<class... Args > static constexpr auto
//   construct(Allocator& a, T* p, Args&&... args) -> void; static constexpr
//   auto destroy(Allocator& a, T* p) -> void;
// };
//
// template <class T>
// constexpr auto
// allocator_traits<spoor::runtime::buffer::BufferSlicePoolAllocator<T>>::allocate(Allocator&,
// const size_t size) -> T*{
//   return static_cast<T*>(::operator new(size * sizeof(T)));
// }
//
// template <class T>
// constexpr auto
// allocator_traits<spoor::runtime::buffer::BufferSlicePoolAllocator<T>>::deallocate(Allocator&,
// T* ptr, const size_t) -> void {
//   ::operator delete(ptr);
// }
//
// }  // namespace std

#endif
