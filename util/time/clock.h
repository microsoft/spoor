// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <chrono>

namespace util::time {

template <class ChronoClock>
class Clock;

using SystemClock = Clock<std::chrono::system_clock>;
using SteadyClock = Clock<std::chrono::steady_clock>;

template <class ChronoClock>
class Clock {
 public:
  constexpr Clock() = default;
  constexpr Clock(const Clock&) = default;
  constexpr Clock(Clock&&) noexcept = default;
  constexpr auto operator=(const Clock&) -> Clock& = default;
  constexpr auto operator=(Clock&&) noexcept -> Clock& = default;
  virtual ~Clock() = default;

  [[nodiscard]] virtual constexpr auto Now() const
      -> std::chrono::time_point<ChronoClock>;
};

template <class ChronoClock>
constexpr auto Clock<ChronoClock>::Now() const
    -> std::chrono::time_point<ChronoClock> {
  return ChronoClock::now();
}

}  // namespace util::time
