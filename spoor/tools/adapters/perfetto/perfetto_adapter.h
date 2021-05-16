// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <string_view>
#include <vector>

#include "protos/perfetto/trace/trace.pb.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/runtime/trace/trace.h"
#include "util/numeric.h"

namespace spoor::tools::adapters::perfetto {

constexpr uint32 kTrustedPacketSequenceId{1};
constexpr std::string_view kPerfettoFileExtension{"perfetto"};

auto SpoorTraceToPerfettoTrace(
    const std::vector<runtime::trace::TraceFile>& trace_files,
    const instrumentation::symbols::Symbols& symbols)
    -> ::perfetto::protos::Trace;

}  // namespace spoor::tools::adapters::perfetto
