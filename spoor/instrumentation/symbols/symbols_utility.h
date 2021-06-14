// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "gsl/gsl"
#include "spoor/instrumentation/symbols/symbols.pb.h"

namespace spoor::instrumentation::symbols {

auto ReduceSymbols(gsl::not_null<Symbols*> source,
                   gsl::not_null<Symbols*> destination) -> void;

}  // namespace spoor::instrumentation::symbols
