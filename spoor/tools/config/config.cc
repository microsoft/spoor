// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/tools/config/config.h"

namespace spoor::tools::config {

auto operator==(const Config& lhs, const Config& rhs) -> bool {
  return lhs.output_file == rhs.output_file &&
         lhs.output_format == rhs.output_format;
}

}  // namespace spoor::tools::config
