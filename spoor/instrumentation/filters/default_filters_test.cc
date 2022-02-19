// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/filters/default_filters.h"

#include "gtest/gtest.h"
#include "spoor/instrumentation/filters/filters.h"

namespace {

using spoor::instrumentation::filters::DefaultFilters;
using spoor::instrumentation::filters::Filters;
using spoor::instrumentation::filters::FunctionInfo;

TEST(DefaultFilters, GetDefaults) {  // NOLINT
  const Filters default_filters{DefaultFilters()};
  const FunctionInfo function_info{
      .source_file_path = "/path/to/config.cc",
      .demangled_name = "spoor::runtime::ConfigFilePath()",
      .linkage_name = "__ZN5spoor7runtime14ConfigFilePathEv",
      .ir_instruction_count = 0,
  };
  ASSERT_FALSE(default_filters.InstrumentFunction(function_info).instrument);
}

}  // namespace
