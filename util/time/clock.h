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
  Clock() = default;
  Clock(const Clock&) = default;
  Clock(Clock&&) noexcept = default;
  auto operator=(const Clock&) -> Clock& = default;
  auto operator=(Clock&&) noexcept -> Clock& = default;
  virtual ~Clock() = default;

  [[nodiscard]] virtual auto Now() const
      -> std::chrono::time_point<ChronoClock>;
};

template <class ChronoClock>
auto Clock<ChronoClock>::Now() const -> std::chrono::time_point<ChronoClock> {
  return ChronoClock::now();
}

}  // namespace util::time
