#ifndef SPOOR_UTIL_MEMORY_PTR_OWNER_H_
#define SPOOR_UTIL_MEMORY_PTR_OWNER_H_

#include "util/result.h"

namespace util::memory {

template <class T>
class OwnedPtr;

template <class T>
class PtrOwner {
 public:
  enum class Error;

  using Result = util::result::Result<util::result::Void, Error>;

  // TODO do we need to assert that Ptr owner is not movable
  // if they are, the ownership will be messed up

  enum class Error {
    kDoesNotOwnPtr,
  };

  PtrOwner() = default;
  PtrOwner(const PtrOwner&) = default;
  PtrOwner(PtrOwner&&) noexcept = delete;
  auto operator=(const PtrOwner&) -> PtrOwner& = default;
  auto operator=(PtrOwner&&) noexcept -> PtrOwner& = delete;
  virtual ~PtrOwner() = default;

  virtual auto Return(OwnedPtr<T>&& ptr) -> Result = 0;

 protected:
  friend class OwnedPtr<T>;

  virtual auto Return(T* t) -> Result = 0;
};

}  // namespace util::memory

#endif
