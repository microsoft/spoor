#ifndef SPOOR_UTIL_STRINGS_H_
#define SPOOR_UTIL_STRINGS_H_

#include <sstream>
#include <string>

namespace util::strings {

template <class Iterable>
auto Join(const Iterable& collection, const std::string_view delimiter)
    -> std::string {
  std::stringstream joined{};
  for (auto item = collection.begin(); item != collection.end(); ++item) {
    joined << *item;
    if (item != std::prev(collection.end())) joined << delimiter;
  }
  return joined.str();
}

}  // namespace util::strings

#endif
