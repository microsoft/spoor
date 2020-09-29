#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/strings/str_join.h"
#include "external/com_bazelbuild_bazel/src/main/protobuf/extra_actions_base.pb.h"
#include "google/protobuf/stubs/common.h"
#include "toolchain/compilation_database/compilation_database_util.h"
#include "toolchain/compilation_database/compile_commands.pb.h"
#include "util/result.h"

ABSL_FLAG(  // NOLINT
    std::string, extra_action_file, {},
    "Path to an extra action file provided by the Bazel action listener.");

ABSL_FLAG(  // NOLINT
    std::string, output_file, {},
    "Path where the generated compile command should be saved.");

namespace {

using toolchain::compilation_database::ParseExtraActionInfoError;

constexpr std::string_view kUsage{
    "Extract the command to compile each C++ source file from its `CppCompile` "
    "build mnemonic."};

}  // namespace

auto main(int argc, char** argv) -> int {
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  const auto _ =
      gsl::finally([] { google::protobuf::ShutdownProtobufLibrary(); });

  absl::SetProgramUsageMessage(kUsage);
  absl::ParseCommandLine(argc, argv);

  const auto extra_action_file = absl::GetFlag(FLAGS_extra_action_file);
  std::ifstream input_stream{extra_action_file,
                             std::ios::in | std::ios::binary};
  const auto parse_result =
      toolchain::compilation_database::ParseExtraActionInfo(&input_stream);
  if (parse_result.IsErr()) {
    switch (parse_result.Err()) {
      case ParseExtraActionInfoError::kParsingError:
        std::cerr << "Failed to parse the extra action file `"
                  << extra_action_file << "`.\n";
        break;
      case ParseExtraActionInfoError::kExtraActionInfoMissingExtension:
        std::cerr << "Failed to parse the extra action file `"
                  << extra_action_file
                  << "` because it is missing the CppCompileInfo extension.\n";
        break;
    }
    return EXIT_FAILURE;
  }
  const auto& compile_command = parse_result.Ok();

  const auto output_path = absl::GetFlag(FLAGS_output_file);
  std::ofstream output_stream{
      output_path, std::ios::out | std::ios::trunc | std::ios::binary};
  const auto output_result =
      SerializeCompileCommandToOutputStream(compile_command, &output_stream);
  if (output_result.IsErr()) {
    std::cerr
        << "Failed to serialize the compile command to the output file.\n";
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
