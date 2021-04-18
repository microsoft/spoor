// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <istream>
#include <unordered_set>

#include "gsl/gsl"

namespace spoor::instrumentation::support {

auto ReadLinesToSet(gsl::not_null<std::istream*> istream)
    -> std::unordered_set<std::string>;

}  // namespace spoor::instrumentation::support
