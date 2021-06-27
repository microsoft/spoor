// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/tools/serialization/csv/csv.h"

#include <ios>
#include <sstream>
#include <string>

#include "google/protobuf/util/time_util.h"
#include "gtest/gtest.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"

namespace {

using google::protobuf::util::TimeUtil;
using spoor::instrumentation::symbols::Symbols;
using spoor::tools::serialization::csv::SerializeSymbolsToOstreamAsCsv;

TEST(SerializeSymbolsToOstreamAsCsv, SerializesToCsv) {  // NOLINT
  const auto symbols = [&] {
    Symbols symbols{};
    auto& function_symbols_table = *symbols.mutable_function_symbols_table();
    auto& repeated_function_infos_0 = function_symbols_table[15];

    auto* function_a_info = repeated_function_infos_0.add_function_infos();
    function_a_info->set_module_id("module_a");
    function_a_info->set_linkage_name("function_a");
    function_a_info->set_demangled_name("FunctionA(int x, int y)");
    function_a_info->set_file_name("file_a.source");
    function_a_info->set_directory("/path/to/a");
    function_a_info->set_line(1);
    function_a_info->set_ir_instruction_count(100);
    function_a_info->set_instrumented(true);
    function_a_info->set_instrumented_reason("Rule A");
    *function_a_info->mutable_created_at() =
        TimeUtil::NanosecondsToTimestamp(1);

    repeated_function_infos_0.add_function_infos();

    auto& repeated_function_infos_1 = function_symbols_table[16];
    auto* function_c_info = repeated_function_infos_1.add_function_infos();
    function_c_info->set_module_id("module_c");
    function_c_info->set_linkage_name("function_c");
    function_c_info->set_demangled_name("FunctionC()");
    function_c_info->set_file_name("file_c.source");
    function_c_info->set_directory("/path/to/c");
    function_c_info->set_line(42);
    function_c_info->set_ir_instruction_count(200);
    function_c_info->set_instrumented(false);
    function_c_info->set_instrumented_reason("Rule C");
    *function_c_info->mutable_created_at() =
        TimeUtil::NanosecondsToTimestamp(42);

    return symbols;
  }();
  const std::string expected_csv{
      "function_id;module_id;linkage_name;demangled_name;file_name;directory;"
      "line;ir_instruction_count;instrumented;instrumented_reason;created_at\n"
      "0x0000000000000f;module_a;function_a;FunctionA(int x, int y);"
      "file_a.source;/path/to/a;1;100;true;Rule "
      "A;1970-01-01T00:00:00.000000001Z\n"
      "0x0000000000000f;;;;;;;0;false;;1970-01-01T00:00:00Z\n"
      "0x00000000000010;module_c;function_c;FunctionC();file_c.source;"
      "/path/to/c;42;200;false;Rule C;1970-01-01T00:00:00.000000042Z"};
  std::stringstream buffer{};
  const auto result = SerializeSymbolsToOstreamAsCsv(symbols, &buffer);
  ASSERT_TRUE(result.IsOk());
  ASSERT_EQ(buffer.str(), expected_csv);
}

TEST(SerializeSymbolsToOstreamAsCsv, HandlesStreamError) {  // NOLINT
  Symbols symbols{};
  std::stringstream buffer{};
  buffer.setstate(std::ios::failbit);
  const auto result = SerializeSymbolsToOstreamAsCsv(symbols, &buffer);
  ASSERT_TRUE(result.IsErr());
}

}  // namespace
