// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <ostream>
#include <string_view>

#include "gsl/gsl"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "util/result.h"

namespace spoor::tools::serialization::csv {

constexpr std::string_view kCsvFileExtension{"csv"};
// A comma is commonly used to delimit demangled function arguments.
constexpr std::string_view kCsvDelimiter{";"};

auto SerializeSymbolsToOstreamAsCsv(
    const instrumentation::symbols::Symbols& symbols,
    gsl::not_null<std::ostream*> ostream)
    -> util::result::Result<util::result::None, util::result::None>;

}  // namespace spoor::tools::serialization::csv
