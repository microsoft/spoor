// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/symbols/symbols_utility.h"

#include <utility>

#include "gsl/gsl"
#include "spoor/instrumentation/symbols/symbols.pb.h"

namespace spoor::instrumentation::symbols {

auto ReduceSymbols(const SymbolsSourceDestination symbols_source_destination)
    -> void {
  const auto source = symbols_source_destination.source;
  const auto destination = symbols_source_destination.destination;
  auto& source_symbols_table = *source->mutable_function_symbols_table();
  auto& destination_function_symbols_table =
      *destination->mutable_function_symbols_table();
  for (auto& [function_id, source_function_infos] : source_symbols_table) {
    auto& destination_function_infos =
        destination_function_symbols_table[function_id];
    auto* repeated_destination_function_infos =
        destination_function_infos.mutable_function_infos();
    auto& repeated_source_function_infos =
        *source_function_infos.mutable_function_infos();
    std::move(std::begin(repeated_source_function_infos),
              std::end(repeated_source_function_infos),
              RepeatedFieldBackInserter(repeated_destination_function_infos));
  }
  source_symbols_table.clear();
}

}  // namespace spoor::instrumentation::symbols
