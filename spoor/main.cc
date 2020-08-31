#include <iostream>

#include "spoor/lib.h"

auto main() -> int {
  const int a{1};
  const int b{2};
  std::cout << a << " + " << b << " = " << spoor::lib::Add(a, b) << '\n';
}
