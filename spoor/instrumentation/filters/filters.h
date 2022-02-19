// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "spoor/instrumentation/filters/filter.h"

namespace spoor::instrumentation::filters {

class Filters {
 public:
  struct InstrumentFunctionResult {
    bool instrument;
    std::optional<std::string> active_filter_rule_name;
  };

  Filters(std::initializer_list<Filter> filters);
  explicit Filters(std::vector<Filter> filters);

  [[nodiscard]] auto InstrumentFunction(const FunctionInfo& function_info) const
      -> InstrumentFunctionResult;

 private:
  friend auto operator==(const Filters& lhs, const Filters& rhs) -> bool;

  std::vector<Filter> filters_;
};

}  // namespace spoor::instrumentation::filters
