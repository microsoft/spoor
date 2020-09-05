#ifndef SPOOR_GSL_POINTERS_G_
#define SPOOR_GSL_POINTERS_G_

#include <type_traits>

namespace gsl {

template <class T> requires(std::is_pointer_v<T>)
using owner = T;  // NOLINT(readability-identifier-naming)

static_assert(std::is_constructible_v<owner<int*>>);

}  // namespace gsl

#endif
