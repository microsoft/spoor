// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/filters/filters.h"

#include <algorithm>
#include <vector>

#include "spoor/instrumentation/filters/filter.h"

namespace spoor::instrumentation::filters {

Filters::Filters(std::initializer_list<Filter> filters) : filters_{filters} {}

Filters::Filters(std::vector<Filter> filters) : filters_{std::move(filters)} {}

auto Filters::InstrumentFunction(const FunctionInfo& function_info) const
    -> InstrumentFunctionResult {
  std::optional<std::string> active_filter_rule_name{};
  const auto block = std::any_of(
      std::cbegin(filters_), std::cend(filters_), [&](const auto& filter) {
        switch (filter.action) {
          case Filter::Action::kAllow: {
            return false;
          }
          case Filter::Action::kBlock: {
            const auto matches = filter.Matches(function_info);
            if (matches) active_filter_rule_name = filter.rule_name;
            return matches;
          }
        }
      });
  const auto allow = std::any_of(
      std::cbegin(filters_), std::cend(filters_), [&](const auto& filter) {
        switch (filter.action) {
          case Filter::Action::kAllow: {
            const auto matches = filter.Matches(function_info);
            if (matches) active_filter_rule_name = filter.rule_name;
            return matches;
          }
          case Filter::Action::kBlock: {
            return false;
          }
        }
      });
  return {
      .instrument = !block || allow,
      .active_filter_rule_name = active_filter_rule_name,
  };
}

auto operator==(const Filters& lhs, const Filters& rhs) -> bool {
  return lhs.filters_ == rhs.filters_;
}

}  // namespace spoor::instrumentation::filters
