// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/filters/filters_file_reader.h"

#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "gsl/gsl"
#include "spoor/instrumentation/filters/filters.h"
#include "tomlplusplus/toml.h"

namespace spoor::instrumentation::filters {

FiltersFileReader::FiltersFileReader(Options options)
    : options_{std::move(options)} {}

auto FiltersFileReader::Read(const std::filesystem::path& file_path) const
    -> Result {
  auto& file = *options_.file_reader;
  file.Open(file_path);
  if (!file.IsOpen()) {
    return Error{
        .type = Error::Type::kFailedToOpenFile,
        .message = absl::StrFormat(
            "Failed to open the file \"%s\" for reading.", file_path),
    };
  }
  auto finally = gsl::finally([&file] { file.Close(); });

  auto toml_result = toml::parse(file.Istream());
  if (!toml_result) {
    return Error{
        .type = Error::Type::kMalformedFile,
        .message = std::string{toml_result.error().description()},
    };
  }
  const auto table = std::move(toml_result).table();

  std::vector<Filter> filters{};
  for (const auto& [key, node] : table) {
    const auto action = kActions.FirstValueForKey(key);
    if (!action.has_value()) {
      return Error{
          .type = Error::Type::kUnknownNode,
          .message = absl::StrFormat("Unknown filter type \"%s\".", key),
      };
    }

    const auto* array = node.as_array();
    if (array == nullptr) {
      return Error{
          .type = Error::Type::kMalformedNode,
          .message = absl::StrFormat(
              "Malformed node \"%s\" is not a list of filters.", key),
      };
    }
    for (const auto& element : *array) {
      const auto* table = element.as_table();
      if (table == nullptr) {
        return Error{
            .type = Error::Type::kMalformedNode,
            .message = absl::StrFormat(
                "Malformed node \"%s\" is not a list of filters.", key),
        };
      }

      for (const auto& [rule, _] : *table) {
        if (std::find(std::cbegin(kFilterKeys), std::cend(kFilterKeys), rule) ==
            std::cend(kFilterKeys)) {
          return Error{
              .type = Error::Type::kUnknownNode,
              .message = absl::StrFormat(R"(Unknown rule "%s" in filter "%s".)",
                                         rule, key),
          };
        }
      }

      std::optional<std::string> rule_name{};
      const auto* table_rule_name = table->get(kRuleNameKey);
      if (table_rule_name != nullptr) {
        rule_name = table_rule_name->value<std::string>();
      }

      std::optional<std::string> source_file_path{};
      const auto* table_source_file_path = table->get(kSourceFilePathKey);
      if (table_source_file_path != nullptr) {
        source_file_path = table_source_file_path->value<std::string>();
      }

      std::optional<std::string> function_demangled_name{};
      const auto* table_function_demangled_name =
          table->get(kFunctionDemangledNameKey);
      if (table_function_demangled_name != nullptr) {
        function_demangled_name =
            table_function_demangled_name->value<std::string>();
      }

      std::optional<std::string> function_linkage_name{};
      const auto* table_function_linkage_name =
          table->get(kFunctionLinkageNameKey);
      if (table_function_linkage_name != nullptr) {
        function_linkage_name =
            table_function_linkage_name->value<std::string>();
      }

      std::optional<uint32> function_ir_instruction_count_lt{};
      const auto* table_function_ir_instruction_count_lt =
          table->get(kFunctionIrInstructionCountLtKey);
      if (table_function_ir_instruction_count_lt != nullptr) {
        function_ir_instruction_count_lt =
            table_function_ir_instruction_count_lt->value<uint32>();
      }

      std::optional<uint32> function_ir_instruction_count_gt{};
      const auto* table_function_ir_instruction_count_gt =
          table->get(kFunctionIrInstructionCountGtKey);
      if (table_function_ir_instruction_count_gt != nullptr) {
        function_ir_instruction_count_gt =
            table_function_ir_instruction_count_gt->value<uint32>();
      }

      Filter filter{
          .action = action.value(),
          .rule_name = rule_name,
          .source_file_path = source_file_path,
          .function_demangled_name = function_demangled_name,
          .function_linkage_name = function_linkage_name,
          .function_ir_instruction_count_lt = function_ir_instruction_count_lt,
          .function_ir_instruction_count_gt = function_ir_instruction_count_gt,
      };
      filters.emplace_back(std::move(filter));
    }
  }

  return Filters{filters};
}

}  // namespace spoor::instrumentation::filters
