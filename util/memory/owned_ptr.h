#pragma once

#include "util/memory/ptr_owner.h"
#include "util/result.h"

namespace util::memory {

template <class T>
class OwnedPtr {
 public:
  using PtrOwner = PtrOwner<T>;

  OwnedPtr() = delete;
  constexpr OwnedPtr(T* ptr, PtrOwner* owner);
  OwnedPtr(const OwnedPtr&) = delete;
  constexpr OwnedPtr(OwnedPtr&& other) noexcept;
  auto operator=(const OwnedPtr&) -> OwnedPtr& = delete;
  constexpr auto operator=(OwnedPtr&& other) noexcept -> OwnedPtr&;
  ~OwnedPtr();

  [[nodiscard]] auto Ptr() const -> T*;
  [[nodiscard]] auto Take() -> T*;
  [[nodiscard]] constexpr auto Owner() const -> PtrOwner*;

  auto operator*() const -> typename ::std::add_lvalue_reference<T>::type;
  auto operator->() const noexcept -> T*;

 private:
  T* ptr_;
  PtrOwner* owner_;
};

template <class T>
constexpr OwnedPtr<T>::OwnedPtr(T* ptr, PtrOwner* owner)
    : ptr_{ptr}, owner_{owner} {}

template <class T>
constexpr OwnedPtr<T>::OwnedPtr(OwnedPtr&& other) noexcept
    : ptr_{other.ptr_}, owner_{other.owner_} {
  other.ptr_ = nullptr;
  other.owner_ = nullptr;
}

template <class T>
constexpr auto OwnedPtr<T>::operator=(OwnedPtr&& other) noexcept -> OwnedPtr& {
  ptr_ = other.ptr_;
  owner_ = other.owner_;
  other.ptr_ = nullptr;
  other.owner_ = nullptr;
}

template <class T>
OwnedPtr<T>::~OwnedPtr() {
  if (owner_ != nullptr) {
    auto owner = owner_;
    owner->Return(Take());
  }
}

template <class T>
auto OwnedPtr<T>::Ptr() const -> T* {
  return ptr_;
}

template <class T>
auto OwnedPtr<T>::Take() -> T* {
  auto ptr = ptr_;
  ptr_ = nullptr;
  owner_ = nullptr;
  return ptr;
}

template <class T>
constexpr auto OwnedPtr<T>::Owner() const -> PtrOwner* {
  return owner_;
}

template <class T>
auto OwnedPtr<T>::operator*() const ->
    typename std::add_lvalue_reference<T>::type {
  return *ptr_;
}

template <class T>
auto OwnedPtr<T>::operator->() const noexcept -> T* {
  return ptr_;
}

}  // namespace util::memory
