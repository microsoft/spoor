// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/tools/serialization/serialize.h"

#include <filesystem>
#include <ostream>
#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "gsl/gsl"
#include "spoor/instrumentation/instrumentation.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/runtime/trace/trace.h"
#include "spoor/tools/adapters/perfetto/perfetto_adapter.h"
#include "util/result.h"

namespace spoor::tools::serialization {

using adapters::perfetto::kPerfettoFileExtension;
using adapters::perfetto::SpoorTraceToPerfettoTrace;
using instrumentation::kSymbolsFileExtension;
using instrumentation::symbols::Symbols;
using runtime::trace::TraceFile;
using Result = util::result::Result<util::result::None, util::result::None>;

auto OutputFormatFromConfig(const config::Config& config)
    -> util::result::Result<OutputFormat, OutputFormatFromConfigError> {
  switch (config.output_format) {
    case config::OutputFormat::kAutomatic: {
      const std::filesystem::path path{config.output_file};
      if (path.extension() == absl::StrCat(".", kPerfettoFileExtension)) {
        return OutputFormat::kPerfetto;
      }
      if (path.extension() == absl::StrCat(".", kSymbolsFileExtension)) {
        return OutputFormat::kSpoorSymbols;
      }
      return OutputFormatFromConfigError::kUnknownFileExtension;
    }
    case config::OutputFormat::kPerfetto: {
      return OutputFormat::kPerfetto;
    }
    case config::OutputFormat::kSpoorSymbols: {
      return OutputFormat::kSpoorSymbols;
    }
  }
}

auto SerializeToOstream(const std::vector<TraceFile>& trace_files,
                        const Symbols& symbols,
                        const OutputFormat output_format,
                        gsl::not_null<std::ostream*> ostream)
    -> util::result::Result<util::result::None, util::result::None> {
  const auto success = [&] {
    switch (output_format) {
      case OutputFormat::kPerfetto: {
        const auto perfetto_trace =
            SpoorTraceToPerfettoTrace(trace_files, symbols);
        return perfetto_trace.SerializeToOstream(ostream);
      }
      case OutputFormat::kSpoorSymbols: {
        return symbols.SerializeToOstream(ostream);
      }
    }
  }();
  return success ? Result::Ok({}) : Result::Err({});
}

}  // namespace spoor::tools::serialization
