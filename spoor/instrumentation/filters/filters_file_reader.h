// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <filesystem>
#include <memory>
#include <string_view>

#include "spoor/instrumentation/filters/filters.h"
#include "spoor/instrumentation/filters/filters_reader.h"
#include "util/file_system/file_reader.h"
#include "util/flat_map/flat_map.h"
#include "util/result.h"

namespace spoor::instrumentation::filters {

constexpr std::string_view kAllowKey{"allow"};
constexpr std::string_view kBlockKey{"block"};
constexpr std::string_view kRuleNameKey{"rule_name"};
constexpr std::string_view kSourceFilePathKey{"source_file_path"};
constexpr std::string_view kFunctionDemangledNameKey{"function_demangled_name"};
constexpr std::string_view kFunctionLinkageNameKey{"function_linkage_name"};
constexpr std::string_view kFunctionIrInstructionCountLtKey{
    "function_ir_instruction_count_lt"};
constexpr std::string_view kFunctionIrInstructionCountGtKey{
    "function_ir_instruction_count_gt"};

constexpr util::flat_map::FlatMap<std::string_view, Filter::Action, 2> kActions{
    {
        {kAllowKey, Filter::Action::kAllow},
        {kBlockKey, Filter::Action::kBlock},
    }};
constexpr std::array<std::string_view, 6> kFilterKeys{{
    kRuleNameKey,
    kSourceFilePathKey,
    kFunctionDemangledNameKey,
    kFunctionLinkageNameKey,
    kFunctionIrInstructionCountLtKey,
    kFunctionIrInstructionCountGtKey,
}};

class FiltersFileReader : public FiltersReader {
 public:
  struct Options {
    std::unique_ptr<util::file_system::FileReader> file_reader;
  };

  explicit FiltersFileReader(Options options);

  [[nodiscard]] auto Read(const std::filesystem::path& file_path) const
      -> Result override;

 private:
  Options options_;
};

}  // namespace spoor::instrumentation::filters
