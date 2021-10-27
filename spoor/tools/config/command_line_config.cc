// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/tools/config/command_line_config.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/ascii.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"
#include "spoor/tools/config/config.h"
#include "util/env/env.h"
#include "util/file_system/util.h"

ABSL_FLAG(  // NOLINT
    std::string, output_file,
    std::string{spoor::tools::config::kOutputFileDefaultValue},
    spoor::tools::config::kOutputFileDoc);
ABSL_FLAG(  // NOLINT
    spoor::tools::config::OutputFormat, output_format,
    spoor::tools::config::kOutputFormatDefaultValue,
    absl::StrFormat(spoor::tools::config::kOutputFormatDoc,
                    absl::StrJoin(spoor::tools::config::kOutputFormats.Keys(),
                                  ", ")));

namespace spoor::tools::config {

auto AbslParseFlag(absl::string_view user_key, OutputFormat* output_format,
                   std::string* error) -> bool {
  std::string key{user_key};
  absl::AsciiStrToLower(&key);
  const auto option = kOutputFormats.FirstValueForKey(key);
  if (option.has_value()) {
    *output_format = option.value();
    return true;
  }
  *error = absl::StrFormat("Unknown output format '%s'. Options: %s.", user_key,
                           absl::StrJoin(kOutputFormats.Keys(), ", "));
  return false;
}

auto AbslUnparseFlag(const OutputFormat output_format) -> std::string {
  const auto fallback = absl::StrCat(output_format);
  const auto value =
      kOutputFormats.FirstKeyForValue(output_format).value_or(fallback);
  return std::string{value};
}

auto ConfigFromCommandLine(int argc, char** argv,
                           const util::env::StdGetEnv& get_env)
    -> std::pair<Config, std::vector<char*>> {
  absl::SetFlag(&FLAGS_output_file, kOutputFileDefaultValue);
  absl::SetFlag(&FLAGS_output_format, kOutputFormatDefaultValue);
  auto positional_args = absl::ParseCommandLine(argc, argv);
  const auto output_file = absl::GetFlag(FLAGS_output_file);
  Config config{
      .output_file = util::file_system::ExpandTilde(output_file, get_env),
      .output_format = absl::GetFlag(FLAGS_output_format),
  };
  return std::make_pair(std::move(config), std::move(positional_args));
}

}  // namespace spoor::tools::config
