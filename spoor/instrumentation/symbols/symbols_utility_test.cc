// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/symbols/symbols_utility.h"

#include "google/protobuf/util/message_differencer.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"

namespace {

using google::protobuf::util::MessageDifferencer;
using spoor::instrumentation::symbols::ReduceSymbols;
using spoor::instrumentation::symbols::Symbols;

TEST(ReduceSymbols, ReducesUniqueSymbols) {  // NOLINT
  auto source_symbols = [] {
    Symbols symbols{};
    auto& function_symbols_table = *symbols.mutable_function_symbols_table();
    auto& function_infos_0 = function_symbols_table[0];
    auto& function_info_a = *function_infos_0.add_function_infos();
    function_info_a.set_module_id("a");
    auto& function_infos_1 = function_symbols_table[1];
    auto& function_info_b = *function_infos_1.add_function_infos();
    function_info_b.set_module_id("b");
    return symbols;
  }();
  auto destination_symbols = [] {
    Symbols symbols{};
    auto& function_symbols_table = *symbols.mutable_function_symbols_table();
    auto& function_infos_1 = function_symbols_table[1];
    auto& function_info_b = *function_infos_1.add_function_infos();
    function_info_b.set_module_id("b");
    auto& function_info_c = *function_infos_1.add_function_infos();
    function_info_c.set_module_id("c");
    auto& function_infos_2 = function_symbols_table[2];
    auto& function_info_d = *function_infos_2.add_function_infos();
    function_info_d.set_module_id("d");
    return symbols;
  }();
  const Symbols expected_source_symbols{};
  const auto expected_destination_symbols = [] {
    Symbols symbols{};
    auto& function_symbols_table = *symbols.mutable_function_symbols_table();
    auto& function_infos_0 = function_symbols_table[0];
    auto& function_info_a = *function_infos_0.add_function_infos();
    function_info_a.set_module_id("a");
    auto& function_infos_1 = function_symbols_table[1];
    auto& function_info_b0 = *function_infos_1.add_function_infos();
    function_info_b0.set_module_id("b");
    auto& function_info_c = *function_infos_1.add_function_infos();
    function_info_c.set_module_id("c");
    auto& function_info_b1 = *function_infos_1.add_function_infos();
    function_info_b1.set_module_id("b");
    auto& function_infos_2 = function_symbols_table[2];
    auto& function_info_d = *function_infos_2.add_function_infos();
    function_info_d.set_module_id("d");
    return symbols;
  }();
  ReduceSymbols(&source_symbols, &destination_symbols);
  ASSERT_TRUE(
      MessageDifferencer::Equals(source_symbols, expected_source_symbols));
  ASSERT_TRUE(MessageDifferencer::Equals(destination_symbols,
                                         expected_destination_symbols));
}

}  // namespace
