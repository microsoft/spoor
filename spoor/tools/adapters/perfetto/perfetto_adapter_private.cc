// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/tools/adapters/perfetto/perfetto_adapter_private.h"

#include "city_hash/city.h"
#include "spoor/runtime/trace/trace.h"
#include "util/numeric.h"

namespace spoor::tools::adapters::perfetto::internal {

using spoor::runtime::trace::ProcessId;
using spoor::runtime::trace::ThreadId;

auto TrackUuid(const ProcessId process_id, const ThreadId thread_id) -> uint64 {
  const auto process_id_hash = CityHash64(
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const char*>(&process_id), sizeof(process_id));
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return CityHash64WithSeed(reinterpret_cast<const char*>(&thread_id),
                            sizeof(thread_id), process_id_hash);
}

}  // namespace spoor::tools::adapters::perfetto::internal
