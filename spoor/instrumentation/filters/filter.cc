// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/filters/filter.h"

#include <regex>

namespace spoor::instrumentation::filters {

auto Filter::Matches(const FunctionInfo& function_info) const -> bool {
  auto matches{true};
  if (source_file_path.has_value()) {
    const std::regex pattern{source_file_path.value()};
    matches &= std::regex_match(function_info.source_file_path, pattern);
  }
  if (function_demangled_name.has_value()) {
    const std::regex pattern{function_demangled_name.value()};
    matches &= std::regex_match(function_info.demangled_name, pattern);
  }
  if (function_linkage_name.has_value()) {
    const std::regex pattern{function_linkage_name.value()};
    matches &= std::regex_match(function_info.linkage_name, pattern);
  }
  if (function_ir_instruction_count_lt.has_value()) {
    matches &= function_info.ir_instruction_count <
               function_ir_instruction_count_lt.value();
  }
  if (function_ir_instruction_count_gt.has_value()) {
    matches &= function_ir_instruction_count_gt.value() <
               function_info.ir_instruction_count;
  }
  return matches;
}

auto operator==(const Filter& lhs, const Filter& rhs) -> bool {
  return lhs.action == rhs.action && lhs.rule_name == rhs.rule_name &&
         lhs.source_file_path == rhs.source_file_path &&
         lhs.function_demangled_name == rhs.function_demangled_name &&
         lhs.function_linkage_name == rhs.function_linkage_name &&
         lhs.function_ir_instruction_count_lt ==
             rhs.function_ir_instruction_count_lt &&
         lhs.function_ir_instruction_count_gt ==
             rhs.function_ir_instruction_count_gt;
}

}  // namespace spoor::instrumentation::filters
