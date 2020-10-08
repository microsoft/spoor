#include <chrono>

#include "gmock/gmock.h"
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

}  // namespace util::time::testing
