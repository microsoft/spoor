// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "city_hash/city.h"
#include "spoor/runtime/trace/trace.h"
#include "util/numeric.h"

namespace spoor::tools::adapters::perfetto::internal {

auto TrackUuid(runtime::trace::ProcessId process_id,
               runtime::trace::ThreadId thread_id) -> uint64;

}  // namespace spoor::tools::adapters::perfetto::internal
