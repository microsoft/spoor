// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <functional>
#include <thread>
#include <vector>

#include "benchmark/benchmark.h"
#include "external/com_google_benchmark/_virtual_includes/benchmark/benchmark/benchmark.h"
#include "gsl/gsl"
#include "spoor/runtime/flush_queue/black_hole_flush_queue.h"
#include "spoor/runtime/runtime_manager/runtime_manager.h"
#include "spoor/runtime/trace/trace.h"
#include "util/numeric.h"
#include "util/time/clock.h"

namespace {

using spoor::runtime::flush_queue::BlackHoleFlushQueue;
using spoor::runtime::runtime_manager::RuntimeManager;
using spoor::runtime::trace::Event;

constexpr bool kFlushAllEvents{true};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::vector<int64> kFibonacciArguments{0, 10, 20, 30};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::vector<int64> kThreadCounts{1, 2, 4, 16};
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::vector<std::vector<int64>> kNAndThreadCount{kFibonacciArguments,
                                                       kThreadCounts};

// NOLINTNEXTLINE(misc-no-recursion)
auto FibonacciUninstrumented(const uint64 n,
                             gsl::not_null<RuntimeManager*> runtime_manager)
    -> uint64 {
  if (n < 2) return n;
  return FibonacciUninstrumented(n - 1, runtime_manager) +
         FibonacciUninstrumented(n - 2, runtime_manager);
}

// NOLINTNEXTLINE(misc-no-recursion)
auto FibonacciInstrumented(const uint64 n,
                           gsl::not_null<RuntimeManager*> runtime_manager)
    -> uint64 {
  constexpr uint64 function_id{42};
  runtime_manager->LogEvent(Event::Type::kFunctionEntry, function_id);
  if (n < 2) {
    runtime_manager->LogEvent(Event::Type::kFunctionExit, function_id);
    return n;
  }
  const auto result = FibonacciInstrumented(n - 1, runtime_manager) +
                      FibonacciInstrumented(n - 2, runtime_manager);
  runtime_manager->LogEvent(Event::Type::kFunctionExit, function_id);
  return result;
}

// This linear-time implementation is simple, exact, and performant enough for
// our needs. A floating point closed-form solution also exists, as do
// techniques to do the computation at compile-time.
auto FibonacciEfficient(const uint64 n) -> uint64 {
  uint64 a{1};
  uint64 b{1};
  for (uint64 i{1}; i < n; ++i) {
    const auto c = a + b;
    a = b;
    b = c;
  }
  return b;
}

auto FibonacciRecursiveCallCount(const uint64 n) -> uint64 {
  // The number of function calls needed to recursively compute the nth
  // Fibonacci number is linearly proportional to the nth Fibonacci number.
  //
  // let f(n) == Fibonacci(n)
  // let g(n) == FibonacciRecursiveCallCount(n)
  //
  // Proof that g(n) = 2 * f(n) - 1
  //
  // n == 0:    g(0) = 1
  // n == 1:    g(1) = 1
  // Otherwise: g(n) = 1 + g(n - 1) + g(n - 2)
  //                 ^ This function call plus the number of function calls to
  //                   compute f(n - 1) plus the number of function calls to
  //                   compute f(n - 2).
  //
  // Base case (n == 0 and n == 1)
  // g(n) = 1
  //      = 2 * 1 - 1
  //      = 2 * f(n) - 1
  //
  // Inductive step (n >= 2)
  // g(n + 1) = 1 + g(n) + g(n - 1)
  //          = 1 + (2 * f(n) - 1) + (2 * f(n - 1) - 1)
  //          = 2 * f(n) + 2 * f(n - 1) - 1
  //          = 2 * f(n + 1) - 1
  return 2 * FibonacciEfficient(n) - 1;
}

// NOLINTNEXTLINE(google-runtime-references)
auto BenchmarkThreadCreation(benchmark::State& state) -> void {
  constexpr uint64 n{0};
  const auto fibonacci_function_calls = FibonacciRecursiveCallCount(n);
  const auto event_count_per_thread = 2 * fibonacci_function_calls;
  const auto thread_count = state.range(0);
  util::time::SteadyClock steady_clock{};
  BlackHoleFlushQueue flush_queue{};
  RuntimeManager runtime_manager{
      {.steady_clock = &steady_clock,
       .flush_queue = &flush_queue,
       .thread_event_buffer_capacity = event_count_per_thread,
       .reserved_pool_capacity = thread_count * event_count_per_thread,
       .reserved_pool_max_slice_capacity = event_count_per_thread,
       .dynamic_pool_capacity = 0,
       .dynamic_pool_max_slice_capacity = 0,
       .dynamic_pool_borrow_cas_attempts = 1,
       .max_buffer_flush_attempts = 1,
       .flush_all_events = kFlushAllEvents}};
  std::vector<std::thread> threads{};
  threads.reserve(thread_count);
  for (auto _ : state) {  // NOLINT(clang-analyzer-deadcode.DeadStores)
    for (auto thread{0}; thread < thread_count; ++thread) {
      threads.emplace_back(FibonacciInstrumented, 0, &runtime_manager);
    }
    std::for_each(std::begin(threads), std::end(threads), [](auto& thread) {
      if (thread.joinable()) thread.join();
    });
    threads.clear();
  }
}

BENCHMARK(BenchmarkThreadCreation)  // NOLINT
    ->ArgsProduct({kThreadCounts});

// NOLINTNEXTLINE(google-runtime-references)
auto BenchmarkFibonacciUninstrumented(benchmark::State& state) -> void {
  const auto n = state.range(0);
  const auto fibonacci_function_calls = FibonacciRecursiveCallCount(n);
  const auto event_count_per_thread = 2 * fibonacci_function_calls;
  const auto thread_count = state.range(1);
  util::time::SteadyClock steady_clock{};
  BlackHoleFlushQueue flush_queue{};
  RuntimeManager runtime_manager{
      {.steady_clock = &steady_clock,
       .flush_queue = &flush_queue,
       .thread_event_buffer_capacity = event_count_per_thread,
       .reserved_pool_capacity = thread_count * event_count_per_thread,
       .reserved_pool_max_slice_capacity = event_count_per_thread,
       .dynamic_pool_capacity = 0,
       .dynamic_pool_max_slice_capacity = 0,
       .dynamic_pool_borrow_cas_attempts = 1,
       .max_buffer_flush_attempts = 1,
       .flush_all_events = kFlushAllEvents}};
  std::vector<std::thread> threads{};
  threads.reserve(thread_count);
  for (auto _ : state) {  // NOLINT(clang-analyzer-deadcode.DeadStores)
    for (auto thread{0}; thread < thread_count; ++thread) {
      threads.emplace_back(FibonacciUninstrumented, n, &runtime_manager);
    }
    std::for_each(std::begin(threads), std::end(threads), [](auto& thread) {
      if (thread.joinable()) thread.join();
    });
  }
}

BENCHMARK(BenchmarkFibonacciUninstrumented)  // NOLINT
    ->ArgsProduct(kNAndThreadCount);

// NOLINTNEXTLINE(google-runtime-references)
auto BenchmarkFibonacciInstrumented(benchmark::State& state) -> void {
  const auto n = state.range(0);
  const auto fibonacci_function_calls = FibonacciRecursiveCallCount(n);
  const auto event_count_per_thread = 2 * fibonacci_function_calls;
  const auto thread_count = state.range(1);
  util::time::SteadyClock steady_clock{};
  BlackHoleFlushQueue flush_queue{};
  RuntimeManager runtime_manager{
      {.steady_clock = &steady_clock,
       .flush_queue = &flush_queue,
       .thread_event_buffer_capacity = event_count_per_thread,
       .reserved_pool_capacity = thread_count * event_count_per_thread,
       .reserved_pool_max_slice_capacity = event_count_per_thread,
       .dynamic_pool_capacity = 0,
       .dynamic_pool_max_slice_capacity = 0,
       .dynamic_pool_borrow_cas_attempts = 1,
       .max_buffer_flush_attempts = 1,
       .flush_all_events = kFlushAllEvents}};
  runtime_manager.Initialize();
  runtime_manager.Enable();
  std::vector<std::thread> threads{};
  threads.reserve(thread_count);
  for (auto _ : state) {  // NOLINT(clang-analyzer-deadcode.DeadStores)
    for (auto thread{0}; thread < thread_count; ++thread) {
      threads.emplace_back(FibonacciInstrumented, n, &runtime_manager);
    }
    std::for_each(std::begin(threads), std::end(threads), [](auto& thread) {
      if (thread.joinable()) thread.join();
    });
    threads.clear();
  }
  runtime_manager.Deinitialize();
}

BENCHMARK(BenchmarkFibonacciInstrumented)  // NOLINT
    ->ArgsProduct(kNAndThreadCount);

}  // namespace

BENCHMARK_MAIN();  // NOLINT
