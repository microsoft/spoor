// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <memory>
#include <utility>
#include <vector>

namespace spoor::common::util {

template <class T, class GetValueFromSource, class Source>
auto ValueFromSourceOrDefault(
    const std::vector<std::unique_ptr<Source>>& sources,
    const GetValueFromSource get_value_from_source, const T& default_value)
    -> T {
  for (const auto& source : sources) {
    if (!source->IsRead()) {
      const auto errors = source->Read();
      static_cast<void>(errors);  // Ignored.
    }
    // `std::bind` permits a cleaner API. The alternative approach using lambdas
    // is much more verbose because the compiler cannot infer the type.
    // NOLINTNEXTLINE(modernize-avoid-bind)
    const auto source_value = std::bind(get_value_from_source, source.get())();
    if (source_value.has_value()) return source_value.value();
  }
  return default_value;
}

}  // namespace spoor::common::util
