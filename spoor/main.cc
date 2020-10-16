#include <iostream>
#include <thread>
#include <vector>

#include "spoor/runtime/runtime.h"
#include "util/numeric.h"

auto main() -> int {
  __spoor_runtime_InitializeRuntime();
  __spoor_runtime_EnableRuntime();
  const auto run = [] {
    for (int64 i{0}; i < 100'000; ++i) {
      __spoor_runtime_LogFunctionEntry(i);
      __spoor_runtime_LogFunctionExit(i);
    }
  };
  std::vector<std::thread> threads{};
  threads.reserve(10);
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back(std::thread{run});
  }
  for (auto& thread : threads) {
    if (thread.joinable()) thread.join();
  }
  __spoor_runtime_DeinitializeRuntime();
  std::cerr << "done\n";
}
