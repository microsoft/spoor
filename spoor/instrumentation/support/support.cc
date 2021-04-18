// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/support/support.h"

#include <algorithm>
#include <istream>
#include <unordered_set>

#include "gsl/gsl"

namespace spoor::instrumentation::support {

auto ReadLinesToSet(gsl::not_null<std::istream*> istream)
    -> std::unordered_set<std::string> {
  std::unordered_set<std::string> file_lines{};
  std::copy(std::istream_iterator<std::string>(*istream),
            std::istream_iterator<std::string>(),
            std::inserter(file_lines, std::begin(file_lines)));
  return file_lines;
}

}  // namespace spoor::instrumentation::support
