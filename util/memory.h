#ifndef SPOOR_UTIL_MEMORY_H_
#define SPOOR_UTIL_MEMORY_H_

#include <functional>
#include <iostream>  // todo
#include <type_traits>

#include "gsl/pointers.h"
#include "util/result.h"

namespace util::memory {

using util::result::Result;
using util::result::Void;

template <class T, class OwnershipInfoType>
class OwnedPtr;

template <class T, class OwnershipInfo>
class PtrOwner {
 public:
  using OwnedPtr = OwnedPtr<T, OwnershipInfo>;
  enum class Error {
    kDoesNotOwnPtr,
  };

  PtrOwner() = default;
  PtrOwner(const PtrOwner&) = default;
  PtrOwner(PtrOwner&&) noexcept = default;
  auto operator=(const PtrOwner&) -> PtrOwner& = default;
  auto operator=(PtrOwner&&) noexcept -> PtrOwner& = default;
  virtual ~PtrOwner() = default;

  virtual auto Owns(const OwnedPtr& owned_ptr) const -> bool = 0;
  virtual auto Return(gsl::owner<T*> t, const OwnershipInfo& ownership_info)
      -> Result<Void, Error> = 0;
  virtual auto Return(OwnedPtr&& ptr) -> Result<Void, Error> = 0;
};

template <class T, class OwnershipInfoType>
class OwnedPtr {
 public:
  using PtrOwner = PtrOwner<T, OwnershipInfoType>;

  OwnedPtr() = delete;
  OwnedPtr(T* ptr, PtrOwner* owner, OwnershipInfoType ownership_info);
  OwnedPtr(const OwnedPtr&) = delete;
  OwnedPtr(OwnedPtr&& other) noexcept;
  auto operator=(const OwnedPtr&) -> OwnedPtr& = delete;
  auto operator=(OwnedPtr&& other) noexcept -> OwnedPtr&;
  ~OwnedPtr();

  [[nodiscard]] auto Ptr() const -> T*;
  [[nodiscard]] auto Take() -> gsl::owner<T*>;
  [[nodiscard]] auto Owner() const -> const PtrOwner*;
  [[nodiscard]] auto OwnershipInfo() const -> const OwnershipInfoType&;

  auto operator*() const -> typename ::std::add_lvalue_reference<T>::type;
  auto operator->() const noexcept -> T*;

 private:
  gsl::owner<T*> ptr_;
  PtrOwner* owner_;
  OwnershipInfoType ownership_info_;
};

template <class T, class OwnershipInfoType>
OwnedPtr<T, OwnershipInfoType>::OwnedPtr(T* ptr, PtrOwner* owner,
                                         OwnershipInfoType ownership_info)
    : ptr_{ptr}, owner_{owner}, ownership_info_{ownership_info} {}

template <class T, class OwnershipInfoType>
OwnedPtr<T, OwnershipInfoType>::OwnedPtr(OwnedPtr&& other) noexcept
    : ptr_{other.ptr_},
      owner_{other.owner_},
      ownership_info_{std::move(other.ownership_info_)} {
  other.ptr_ = nullptr;
  other.owner_ = nullptr;
}

template <class T, class OwnershipInfoType>
auto OwnedPtr<T, OwnershipInfoType>::operator=(OwnedPtr&& other) noexcept
    -> OwnedPtr& {
  ptr_ = other.ptr_;
  owner_ = other.owner_;
  ownership_info_ = std::move(other.ownership_info_);
  other.ptr_ = nullptr;
  other.owner_ = nullptr;
}

template <class T, class OwnershipInfoType>
OwnedPtr<T, OwnershipInfoType>::~OwnedPtr() {
  if (owner_ != nullptr) owner_->Return(Take(), ownership_info_);
}

template <class T, class OwnershipInfoType>
auto OwnedPtr<T, OwnershipInfoType>::Ptr() const -> T* {
  return ptr_;
}

template <class T, class OwnershipInfoType>
auto OwnedPtr<T, OwnershipInfoType>::Take() -> T* {
  auto ptr = ptr_;
  ptr_ = nullptr;
  owner_ = nullptr;
  ownership_info_ = {};
  return ptr;
}

template <class T, class OwnershipInfoType>
auto OwnedPtr<T, OwnershipInfoType>::Owner() const -> const PtrOwner* {
  return owner_;
}

template <class T, class OwnershipInfoType>
auto OwnedPtr<T, OwnershipInfoType>::OwnershipInfo() const
    -> const OwnershipInfoType& {
  return ownership_info_;
}

template <class T, class OwnershipInfoType>
auto OwnedPtr<T, OwnershipInfoType>::operator*() const ->
    typename std::add_lvalue_reference<T>::type {
  return *ptr_;
}

template <class T, class OwnershipInfoType>
auto OwnedPtr<T, OwnershipInfoType>::operator->() const noexcept -> T* {
  return ptr_;
}

}  // namespace util::memory

#endif
