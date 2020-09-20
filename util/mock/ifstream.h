#ifndef SPOOR_UTIL_MOCK_IFSTREAM_H_
#define SPOOR_UTIL_MOCK_IFSTREAM_H_

#include <string_view>
#include <unordered_map>
#include <utility>

namespace util::mock {

template <bool Exists>
class Ifstream {
 public:
  explicit Ifstream(std::string_view /*unused*/);
  // NOLINTNEXTLINE(readability-identifier-naming)
  [[nodiscard]] auto fail() const -> bool;
};

template <bool Exists>
Ifstream<Exists>::Ifstream(const std::string_view /*unused*/) {}

template <bool Exists>  // NOLINTNEXTLINE(readability-identifier-naming)
auto Ifstream<Exists>::fail() const -> bool {
  return !Exists;
}

}  // namespace util::mock

#endif
