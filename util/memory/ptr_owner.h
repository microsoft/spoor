// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "util/result.h"

namespace util::memory {

template <class T>
class OwnedPtr;

template <class T>
class PtrOwner {
 public:
  using Result = util::result::Result<util::result::None, OwnedPtr<T>>;

  constexpr PtrOwner() = default;
  constexpr PtrOwner(const PtrOwner&) = default;
  PtrOwner(PtrOwner&&) noexcept = delete;
  constexpr auto operator=(const PtrOwner&) -> PtrOwner& = default;
  auto operator=(PtrOwner&&) noexcept -> PtrOwner& = delete;
  virtual ~PtrOwner() = default;

  virtual auto Return(OwnedPtr<T>&& ptr) -> Result = 0;

 protected:
  friend class OwnedPtr<T>;

  using ReturnRawPtrResult =
      util::result::Result<util::result::None, util::result::None>;

  virtual auto Return(T* t) -> ReturnRawPtrResult = 0;
};

}  // namespace util::memory
