// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>

#include "util/numeric.h"

namespace spoor::instrumentation::filters {

struct alignas(128) FunctionInfo {
  std::string source_file_path;
  std::string demangled_name;
  std::string linkage_name;
  int32 ir_instruction_count;
};

struct alignas(128) Filter {
  enum class Action {
    kAllow,
    kBlock,
  };

  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  Action action;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::optional<std::string> rule_name;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::optional<std::string> source_file_path;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::optional<std::string> function_demangled_name;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::optional<std::string> function_linkage_name;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::optional<int32> function_ir_instruction_count_lt;
  // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
  std::optional<int32> function_ir_instruction_count_gt;

  [[nodiscard]] auto Matches(const FunctionInfo& function_info) const -> bool;
};

auto operator==(const Filter& lhs, const Filter& rhs) -> bool;

}  // namespace spoor::instrumentation::filters
