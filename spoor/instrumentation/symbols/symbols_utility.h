// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "gsl/gsl"
#include "spoor/instrumentation/symbols/symbols.pb.h"

namespace spoor::instrumentation::symbols {

struct SymbolsSourceDestination {
  gsl::not_null<Symbols*> source;
  gsl::not_null<Symbols*> destination;
};

auto ReduceSymbols(SymbolsSourceDestination symbols_source_destination) -> void;

}  // namespace spoor::instrumentation::symbols
