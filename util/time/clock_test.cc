#include "util/time/clock.h"

#include <chrono>

#include "gtest/gtest.h"

namespace {

TEST(SteadyClock, Now) {  // NOLINT
  const auto chrono_now = std::chrono::steady_clock::now();
  const util::time::SteadyClock steady_clock{};
  const auto steady_clock_now = steady_clock.Now();
  ASSERT_LE(chrono_now, steady_clock_now);
}

TEST(SystemClock, Now) {  // NOLINT
  const auto chrono_now = std::chrono::system_clock::now();
  const util::time::SystemClock system_clock{};
  const auto system_clock_now = system_clock.Now();
  ASSERT_LE(chrono_now, system_clock_now);
}

}  // namespace
