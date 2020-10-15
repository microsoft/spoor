#include <chrono>

#include "gmock/gmock.h"
#include "util/numeric.h"
#include "util/time/clock.h"

namespace util::time::testing {

template <class ChronoClock>
class ClockMock;

using SystemClockMock = ClockMock<std::chrono::system_clock>;
using SteadyClockMock = ClockMock<std::chrono::steady_clock>;

template <class ChronoClock>
class ClockMock final : public Clock<ChronoClock> {
 public:
  MOCK_METHOD(std::chrono::time_point<ChronoClock>, Now, (), (const, override));
};

template <class ChronoClock>
auto MakeTimePoint(const int64 timestamp_nanoseconds)
    -> std::chrono::time_point<ChronoClock>;

template <class ChronoClock>
auto MakeTimePoint(const int64 timestamp_nanoseconds)
    -> std::chrono::time_point<ChronoClock> {
  const std::chrono::nanoseconds duration_nanoseconds{timestamp_nanoseconds};
  const auto duration =
      std::chrono::duration_cast<typename ChronoClock::duration>(
          duration_nanoseconds);
  // `ChronoClock` might have a different duration than
  // `std::chrono::nanoseconds` resulting in a loss of precision.
  assert(std::chrono::duration_cast<std::chrono::nanoseconds>(duration) ==
         duration_nanoseconds);
  return std::chrono::time_point<ChronoClock>{duration};
}

}  // namespace util::time::testing
