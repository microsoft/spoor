#include <iostream>

#include "spoor/lib.h"

auto main() -> int {
  const int64 a{1};
  const int64 b{2};
  std::cout << a << " + " << b << " = " << spoor::lib::Add(a, b) << '\n';
}
