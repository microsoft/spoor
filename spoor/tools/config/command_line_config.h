// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <string>
#include <utility>
#include <vector>

#include "absl/flags/declare.h"
#include "absl/flags/parse.h"
#include "absl/strings/string_view.h"
#include "spoor/tools/config/config.h"

ABSL_DECLARE_FLAG(std::string, output_file);                           // NOLINT
ABSL_DECLARE_FLAG(spoor::tools::config::OutputFormat, output_format);  // NOLINT

namespace spoor::tools::config {

auto AbslParseFlag(absl::string_view user_key, OutputFormat* output_format,
                   std::string* error) -> bool;
auto AbslUnparseFlag(OutputFormat output_format) -> std::string;

auto ConfigFromCommandLine(int argc, char** argv)
    -> std::pair<Config, std::vector<char*>>;

}  // namespace spoor::tools::config
