// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/filters/default_filters.h"

#include <vector>

#include "spoor/instrumentation/filters/filter.h"

namespace spoor::instrumentation::filters {

auto DefaultFilters() -> std::vector<Filter> {
  // Do not instrument config functions to prevent recursive initialization.
  return {{
      .action = Filter::Action::kBlock,
      .rule_name = "Block config file path configuration function",
      .source_file_path = {},
      .function_demangled_name = R"(^spoor::runtime::ConfigFilePath\(\)$)",
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = {},
  }};
}

}  // namespace spoor::instrumentation::filters
