#ifndef SPOOR_UTIL_MEMORY_H_
#define SPOOR_UTIL_MEMORY_H_

#include <type_traits>
#include <functional>

#include "util/result.h"

namespace util::memory {

using util::result::Result;
using util::result::Void;

template <class T>
class OwnedPtr;

template <class T>
class PtrOwner {
 public:
  enum class Error {
    kDoesNotOwnPtr,
  };

  virtual ~PtrOwner() = default;

  virtual auto Owns(const OwnedPtr<T>& owned_ptr) const -> bool = 0;
  virtual auto Borrow() -> OwnedPtr<T> = 0;
  virtual auto Return(T* ptr) -> Result<Void, Error> = 0;
  virtual auto Return(OwnedPtr<T>&& ptr) -> Result<Void, Error> = 0;
};

template <class T>
class OwnedPtr {
 public:
  OwnedPtr() = delete;
  OwnedPtr(T* ptr, PtrOwner<T>* owner);
  OwnedPtr(const OwnedPtr&) = delete;
  OwnedPtr(OwnedPtr&& other) noexcept;
  auto operator=(const OwnedPtr&) -> OwnedPtr& = delete;
  auto operator=(OwnedPtr&& other) noexcept -> OwnedPtr&;
  ~OwnedPtr();

  [[nodiscard]] auto Ptr() const -> T*;
  [[nodiscard]] auto Owner() const -> PtrOwner<T>*;
  [[nodiscard]] auto Take() -> T*;

  auto operator*() const -> typename ::std::add_lvalue_reference<T>::type;
  auto operator->() const noexcept -> T*;

 private:
  T* ptr_;
  PtrOwner<T>* owner_;
};

template <class T>
OwnedPtr<T>::OwnedPtr(T* ptr, PtrOwner<T>* owner) : ptr_{ptr}, owner_{owner} {}

template <class T>
OwnedPtr<T>::OwnedPtr(OwnedPtr&& other) noexcept {
  ptr_ = other.ptr_;
  owner_ = other.owner_;
  other.ptr_ = nullptr;
  other.owner_ = nullptr;
}

template <class T>
auto OwnedPtr<T>::operator=(OwnedPtr&& other) noexcept -> OwnedPtr& {
  ptr_ = other.ptr_;
  owner_ = other.owner_;
  other.ptr_ = nullptr;
  other.owner_ = nullptr;
}

template <class T>
OwnedPtr<T>::~OwnedPtr() {
  if (owner_ != nullptr) owner_->Return(ptr_);
}

template <class T>
auto OwnedPtr<T>::Ptr() const -> T* {
  return ptr_;
}

template <class T>
auto OwnedPtr<T>::Owner() const -> PtrOwner<T>* {
  return owner_;
}

template <class T>
auto OwnedPtr<T>::Take() -> T* {
  auto ptr = ptr_;
  ptr_ = nullptr;
  owner_ = nullptr;
  return ptr;
}

template <class T>
auto OwnedPtr<T>::operator*() const -> typename ::std::add_lvalue_reference<T>::type {
  return *ptr_;
}

template <class T>
auto OwnedPtr<T>::operator->() const noexcept -> T* {
  return ptr_;
}

}  // namespace util::memory

// namespace std {
// 
// template <class T>
// struct hash<util::memory::OwnedPtr<T>> {
//   std::size_t operator()(const util::memory::OwnedPtr<T>& owned_ptr) const noexcept {
//     std::size_t h1 = std::hash<T>{}(owned_ptr.Ptr());
//     std::size_t h2 = std::hash<T>{}(owned_ptr.Owner());
//     return h1 ^ (h2 << 1);
//   }
// };
// 
// }  // namespace std
// 
// namespace util::memory {
// 
// template <class T>
// auto operator==(const OwnedPtr<T>& lhs, const OwnedPtr<T>& rhs) -> bool {
//   return std::hash<OwnedPtr<T>>{}(lhs) == std::hash<OwnedPtr<T>>{}(rhs);
// }
// 
// }  // namespace util::memory


#endif
