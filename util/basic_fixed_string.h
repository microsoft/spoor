#ifndef SPOOR_UTIL_BASIC_FIXED_STRING_H_
#define SPOOR_UTIL_BASIC_FIXED_STRING_H_

#include <algorithm>
#include <array>
#include <cstddef>

namespace util {

template <class Char, std::size_t Size>
class BasicFixedString {
 public:
  constexpr explicit BasicFixedString(const std::array<Char, Size + 1U>& data) {
    std::copy_n(data, Size + 1U, data_);
  }
  // constexpr explicit BasicFixedString(const Char data) {
  //   data_[0] = data;
  // }

 private:
  std::array<Char, Size + 1U> data_{};
};

}  // namespace util

#endif
