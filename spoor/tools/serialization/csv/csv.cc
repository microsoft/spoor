// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/tools/serialization/csv/csv.h"

#include <algorithm>
#include <array>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "google/protobuf/util/time_util.h"
#include "gsl/gsl"
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/runtime/trace/trace.h"
#include "util/result.h"

namespace spoor::tools::serialization::csv {

using google::protobuf::util::TimeUtil;
using instrumentation::symbols::Symbols;
using spoor::runtime::trace::FunctionId;
using Result = util::result::Result<util::result::None, util::result::None>;

constexpr std::array<std::string_view, 11> kSpoorSymbolsColumns{{
    "function_id",
    "module_id",
    "linkage_name",
    "demangled_name",
    "file_name",
    "directory",
    "line",
    "ir_instruction_count",
    "instrumented",
    "instrumented_reason",
    "created_at",
}};

auto SerializeSymbolsToOstreamAsCsv(const Symbols& symbols,
                                    gsl::not_null<std::ostream*> ostream)
    -> Result {
  *ostream << absl::StrJoin(kSpoorSymbolsColumns, kCsvDelimiter);
  const auto& function_symbols_table = symbols.function_symbols_table();
  std::vector<FunctionId> function_ids{};
  function_ids.reserve(function_symbols_table.size());
  std::transform(
      std::cbegin(function_symbols_table), std::cend(function_symbols_table),
      std::back_inserter(function_ids),
      [](const auto& function_id_function_infos) {
        const auto& [function_id, function_infos] = function_id_function_infos;
        return function_id;
      });
  std::sort(std::begin(function_ids), std::end(function_ids));
  for (const auto function_id : function_ids) {
    const auto& function_infos = function_symbols_table.at(function_id);
    for (const auto& function_info : function_infos.function_infos()) {
      const std::array<std::string, kSpoorSymbolsColumns.size()> row{
          absl::StrFormat("%#016x", function_id),
          function_info.module_id(),
          function_info.linkage_name(),
          function_info.demangled_name(),
          function_info.file_name(),
          function_info.directory(),
          function_info.line() < 1 ? "" : absl::StrCat(function_info.line()),
          absl::StrCat(function_info.ir_instruction_count()),
          function_info.instrumented() ? "true" : "false",
          function_info.instrumented_reason(),
          TimeUtil::ToString(function_info.created_at()),
      };
      *ostream << '\n';
      *ostream << absl::StrJoin(row, kCsvDelimiter);
    }
  }
  return ostream->good() ? Result::Ok({}) : Result::Err({});
}

}  // namespace spoor::tools::serialization::csv
