#ifndef SPOOR_UTIL_MEMORY_PTR_OWNER_H_
#define SPOOR_UTIL_MEMORY_PTR_OWNER_H_

#include <gsl/pointers>

#include "util/result.h"

namespace util::memory {

template <class T>
class OwnedPtr;

template <class T>
class PtrOwner {
 public:
  enum class Error;

  using OwnedPtr = OwnedPtr<T>;
  using Result = util::result::Result<util::result::Void, Error>;

  // TODO do we need to assert that Ptr owner is not copyable or movable
  // if they are, the ownership will be messed up

  enum class Error {
    kDoesNotOwnPtr,
  };

  PtrOwner() = default;
  PtrOwner(const PtrOwner&) = default;
  PtrOwner(PtrOwner&&) noexcept = default;
  auto operator=(const PtrOwner&) -> PtrOwner& = default;
  auto operator=(PtrOwner&&) noexcept -> PtrOwner& = default;
  virtual ~PtrOwner() = default;

  virtual auto Return(gsl::owner<T*> t) -> Result = 0;
  virtual auto Return(OwnedPtr&& ptr) -> Result = 0;
};

}  // namespace util::memory

#endif
