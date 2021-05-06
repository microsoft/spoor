// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <cstdio>

auto Fibonacci(int n) -> int {  // NOLINT(misc-no-recursion)
  if (n < 2) return n;
  return Fibonacci(n - 1) + Fibonacci(n - 2);
}

auto main() -> int {
  printf("%d", Fibonacci(7));  // NOLINT(cppcoreguidelines-pro-type-vararg)
}
