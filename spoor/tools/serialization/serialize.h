// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <array>
#include <ostream>
#include <string>
#include <vector>

#include "gsl/gsl"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/tools/config/config.h"
#include "util/result.h"

namespace spoor::tools::serialization {

enum class OutputFormat {
  kPerfettoProto,
  kSpoorSymbolsProto,
  kSpoorSymbolsCsv,
};

constexpr std::array<OutputFormat, 3> kOutputFormats{{
    OutputFormat::kPerfettoProto,
    OutputFormat::kSpoorSymbolsProto,
    OutputFormat::kSpoorSymbolsCsv,
}};

enum class OutputFormatFromConfigError {
  kUnknownFileExtension,
};

auto OutputFormatFromConfig(const config::Config& config)
    -> util::result::Result<OutputFormat, OutputFormatFromConfigError>;

auto SerializeToOstream(
    const std::vector<runtime::trace::TraceFile>& trace_files,
    const instrumentation::symbols::Symbols& symbols,
    OutputFormat output_format, gsl::not_null<std::ostream*> ostream)
    -> util::result::Result<util::result::None, util::result::None>;

}  // namespace spoor::tools::serialization
