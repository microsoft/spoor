// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/filters/filters.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "gtest/gtest.h"

namespace {

using spoor::instrumentation::filters::Filter;
using spoor::instrumentation::filters::Filters;
using spoor::instrumentation::filters::FunctionInfo;

struct alignas(128) TestCase {
  FunctionInfo function_info;
  bool instrument;
  std::optional<std::string> active_filter_rule_name;
};

auto RunTestCases(const std::vector<TestCase>& test_cases,
                  const Filters& filters) -> void {
  for (const auto& test_case : test_cases) {
    const auto [instrument, active_filter_rule_name] =
        filters.InstrumentFunction(test_case.function_info);
    ASSERT_EQ(instrument, test_case.instrument);
    ASSERT_EQ(active_filter_rule_name, test_case.active_filter_rule_name);
  }
}

TEST(Filters, HandlesEmptyFilterList) {  // NOLINT
  const Filters filters{{}};
  const std::vector<TestCase> test_cases{
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = {},
              },
          .instrument = true,
          .active_filter_rule_name = {},
      },
      {
          .function_info =
              {
                  .source_file_path = "/path/to/source.extension",
                  .demangled_name = "Demangled name",
                  .linkage_name = "linkage_name",
                  .ir_instruction_count = 1'234,
              },
          .instrument = true,
          .active_filter_rule_name = {},
      },
  };
  RunTestCases(test_cases, filters);
}

TEST(Filters, EmptyFilterMatchesEverything) {  // NOLINT
  const Filter block_filter{
      .action = Filter::Action::kBlock,
      .rule_name = "Block everything",
      .source_file_path = {},
      .function_demangled_name = {},
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = {},
  };
  const Filter allow_filter{
      .action = Filter::Action::kAllow,
      .rule_name = "Allow everything",
      .source_file_path = {},
      .function_demangled_name = {},
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = {},
  };
  {
    const Filters filters{block_filter};
    const std::vector<TestCase> test_cases{
        {
            .function_info =
                {
                    .source_file_path = {},
                    .demangled_name = {},
                    .linkage_name = {},
                    .ir_instruction_count = {},
                },
            .instrument = false,
            .active_filter_rule_name = block_filter.rule_name,
        },
        {
            .function_info =
                {
                    .source_file_path = "/path/to/source.extension",
                    .demangled_name = "Demangled name",
                    .linkage_name = "linkage_name",
                    .ir_instruction_count = 1'234,
                },
            .instrument = false,
            .active_filter_rule_name = block_filter.rule_name,
        },
    };
    RunTestCases(test_cases, filters);
  }
  {
    const Filters filters{block_filter, allow_filter};
    const std::vector<TestCase> test_cases{
        {
            .function_info =
                {
                    .source_file_path = {},
                    .demangled_name = {},
                    .linkage_name = {},
                    .ir_instruction_count = {},
                },
            .instrument = true,
            .active_filter_rule_name = allow_filter.rule_name,
        },
        {
            .function_info =
                {
                    .source_file_path = "/path/to/source.extension",
                    .demangled_name = "Demangled name",
                    .linkage_name = "linkage_name",
                    .ir_instruction_count = 1'234,
                },
            .instrument = true,
            .active_filter_rule_name = allow_filter.rule_name,
        },
    };
    RunTestCases(test_cases, filters);
  }
}

TEST(Filters, HandlesSourceFilePathFilter) {  // NOLINT
  const Filter block_filter{
      .action = Filter::Action::kBlock,
      .rule_name = "Block everything",
      .source_file_path = ".*",
      .function_demangled_name = {},
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = {},
  };
  const Filter allow_filter{
      .action = Filter::Action::kAllow,
      .rule_name = "Allow foo",
      .source_file_path = "foo",
      .function_demangled_name = {},
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = {},
  };
  const Filters filters{block_filter, allow_filter};
  const std::vector<TestCase> test_cases{
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = {},
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = "foo",
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = {},
              },
          .instrument = true,
          .active_filter_rule_name = allow_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = "/path/to/bar.extension",
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = {},
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
  };
  RunTestCases(test_cases, filters);
}

TEST(Filters, HandlesFunctionDemangledNameFilter) {  // NOLINT
  const Filter block_filter{
      .action = Filter::Action::kBlock,
      .rule_name = "Block everything",
      .source_file_path = {},
      .function_demangled_name = ".*",
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = {},
  };
  const Filter allow_filter{
      .action = Filter::Action::kAllow,
      .rule_name = "Allow foo",
      .source_file_path = {},
      .function_demangled_name = "[F|f]oo.*",
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = {},
  };
  const Filters filters{block_filter, allow_filter};
  const std::vector<TestCase> test_cases{
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = {},
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = "Foo()",
                  .linkage_name = {},
                  .ir_instruction_count = {},
              },
          .instrument = true,
          .active_filter_rule_name = allow_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = "Bar()",
                  .linkage_name = {},
                  .ir_instruction_count = {},
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
  };
  RunTestCases(test_cases, filters);
}

TEST(Filters, HandlesFunctionLinkageNameFilter) {  // NOLINT
  const Filter block_filter{
      .action = Filter::Action::kBlock,
      .rule_name = "Block everything",
      .source_file_path = {},
      .function_demangled_name = ".*",
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = {},
  };
  const Filter allow_filter{
      .action = Filter::Action::kAllow,
      .rule_name = "Allow foo",
      .source_file_path = {},
      .function_demangled_name = {},
      .function_linkage_name = "foo",
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = {},
  };
  const Filters filters{block_filter, allow_filter};
  const std::vector<TestCase> test_cases{
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = {},
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = "foo",
                  .ir_instruction_count = {},
              },
          .instrument = true,
          .active_filter_rule_name = allow_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = "bar",
                  .ir_instruction_count = {},
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
  };
  RunTestCases(test_cases, filters);
}

TEST(Filters, HandlesFunctionIrInstructionCountLtFilter) {  // NOLINT
  const Filter block_filter{
      .action = Filter::Action::kBlock,
      .rule_name = "Block functions with less than 100 IR instructions.",
      .source_file_path = {},
      .function_demangled_name = {},
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = 100,
      .function_ir_instruction_count_gt = {},
  };
  const Filter allow_filter{
      .action = Filter::Action::kAllow,
      .rule_name = "Allow functions with less than 2 IR instructions.",
      .source_file_path = {},
      .function_demangled_name = {},
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = 2,
      .function_ir_instruction_count_gt = {},
  };
  const Filters filters{block_filter, allow_filter};
  const std::vector<TestCase> test_cases{
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = -1,
              },
          .instrument = true,
          .active_filter_rule_name = allow_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 0,
              },
          .instrument = true,
          .active_filter_rule_name = allow_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 1,
              },
          .instrument = true,
          .active_filter_rule_name = allow_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 2,
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 3,
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 99,
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 100,
              },
          .instrument = true,
          .active_filter_rule_name = {},
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 101,
              },
          .instrument = true,
          .active_filter_rule_name = {},
      },
  };
  RunTestCases(test_cases, filters);
}

TEST(Filters, HandlesFunctionIrInstructionCountGtFilter) {  // NOLINT
  const Filter block_filter{
      .action = Filter::Action::kBlock,
      .rule_name = "Block functions with more than 2 IR instructions.",
      .source_file_path = {},
      .function_demangled_name = {},
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = 2,
  };
  const Filter allow_filter{
      .action = Filter::Action::kAllow,
      .rule_name = "Allow functions with more than 100 IR instructions.",
      .source_file_path = {},
      .function_demangled_name = {},
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = 100,
  };
  const Filters filters{block_filter, allow_filter};
  const std::vector<TestCase> test_cases{
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = -1,
              },
          .instrument = true,
          .active_filter_rule_name = {},
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 0,
              },
          .instrument = true,
          .active_filter_rule_name = {},
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 1,
              },
          .instrument = true,
          .active_filter_rule_name = {},
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 2,
              },
          .instrument = true,
          .active_filter_rule_name = {},
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 3,
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 99,
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 100,
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = {},
                  .ir_instruction_count = 101,
              },
          .instrument = true,
          .active_filter_rule_name = allow_filter.rule_name,
      },
  };
  RunTestCases(test_cases, filters);
}

TEST(Filters, FilterRulesUseLogialConjunction) {  // NOLINT
  const Filter block_filter{
      .action = Filter::Action::kBlock,
      .rule_name = "Block `foo` functions with less than 10 IR instructions.",
      .source_file_path = {},
      .function_demangled_name = {},
      .function_linkage_name = ".*foo.*",
      .function_ir_instruction_count_lt = 10,
      .function_ir_instruction_count_gt = {},
  };
  const Filters filters{block_filter};
  const std::vector<TestCase> test_cases{
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = "foo",
                  .ir_instruction_count = 5,
              },
          .instrument = false,
          .active_filter_rule_name = block_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = "foo",
                  .ir_instruction_count = 100,
              },
          .instrument = true,
          .active_filter_rule_name = {},
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = "bar",
                  .ir_instruction_count = 5,
              },
          .instrument = true,
          .active_filter_rule_name = {},
      },
  };
  RunTestCases(test_cases, filters);
}

TEST(Filters, FiltersUseLogialDisjunction) {  // NOLINT
  const Filter block_foo_filter{
      .action = Filter::Action::kBlock,
      .rule_name = "Block `foo` functions.",
      .source_file_path = {},
      .function_demangled_name = {},
      .function_linkage_name = ".*foo.*",
      .function_ir_instruction_count_lt = {},
      .function_ir_instruction_count_gt = {},
  };
  const Filter block_small_functions_filter{
      .action = Filter::Action::kBlock,
      .rule_name = "Block functions with less than 10 IR instructions.",
      .source_file_path = {},
      .function_demangled_name = {},
      .function_linkage_name = {},
      .function_ir_instruction_count_lt = 10,
      .function_ir_instruction_count_gt = {},
  };
  const Filters filters{block_foo_filter, block_small_functions_filter};
  const std::vector<TestCase> test_cases{
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = "foo",
                  .ir_instruction_count = 5,
              },
          .instrument = false,
          .active_filter_rule_name = block_foo_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = "foo",
                  .ir_instruction_count = 100,
              },
          .instrument = false,
          .active_filter_rule_name = block_foo_filter.rule_name,
      },
      {
          .function_info =
              {
                  .source_file_path = {},
                  .demangled_name = {},
                  .linkage_name = "bar",
                  .ir_instruction_count = 5,
              },
          .instrument = false,
          .active_filter_rule_name = block_small_functions_filter.rule_name,
      },
  };
  RunTestCases(test_cases, filters);
}

}  // namespace
