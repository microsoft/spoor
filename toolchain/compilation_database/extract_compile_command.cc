
// TODO: Do we need all these headers?
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_join.h"
#include "external/com_bazelbuild_bazel/src/main/protobuf/extra_actions_base.pb.h"
#include "google/protobuf/stubs/common.h"
#include "toolchain/compilation_database/compile_commands.pb.h"
#include "util/result.h"

ABSL_FLAG(
    std::string, extra_action_file_path, "",
    "Path to an extra action file provided by the Bazel action listener.");

// DEFINE_validator(  // NOLINT
//     extra_action_file, &util::flags::ValidateInputFilePath);

ABSL_FLAG(
    std::string, output_path, "",
    "Path where the generated compile command should be saved.");

// DEFINE_validator(  // NOLINT
//     output, &util::flags::ValidateOutputFilePath);

namespace {

using toolchain::compilation_database::CompileCommand;

enum class ParseExtraActionInfoError {
  kParsingError,
  kExtraActionInfoMissingExtension,
};

// const char* const kExtractCompileCommandVersion = "1.0.0";
// const char* const kUsage =
//     "Extract the command to compile each C++ source file from its `CppCompile` "
//     "build mnemonic.";

auto ParseExtraActionInfo(const std::string& path)
    -> util::result::Result<CompileCommand, ParseExtraActionInfoError> {
  std::ifstream input_stream{path, std::ios::in | std::ios::binary};
  blaze::ExtraActionInfo extra_action_info{};
  const auto success = extra_action_info.ParseFromIstream(&input_stream);
  if (!success) return ParseExtraActionInfoError::kParsingError;

  if (!extra_action_info.HasExtension(
          blaze::CppCompileInfo::cpp_compile_info)) {
    return ParseExtraActionInfoError::kExtraActionInfoMissingExtension;
  }

  const auto compile_info =
      extra_action_info.GetExtension(blaze::CppCompileInfo::cpp_compile_info);

  CompileCommand compile_command{};
  compile_command.set_file(compile_info.source_file());

  std::vector<std::string> arguments{};
  arguments.reserve(1 + compile_info.compiler_option().size());
  arguments.push_back(compile_info.tool());
  arguments.insert(arguments.end(), compile_info.compiler_option().begin(),
                   compile_info.compiler_option().end());

  const auto command = absl::StrJoin(arguments, " ");
  compile_command.set_command(command);

  return compile_command;
}

}  // namespace

auto main(int argc, char** argv) -> int {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // absl::SetVersionString(kExtractCompileCommandVersion);
  // absl::SetUsageMessage(kUsage);
  absl::ParseCommandLine(argc, argv);

  const auto result = ParseExtraActionInfo(
      absl::GetFlag(FLAGS_extra_action_file_path));
  if (!result.IsOk()) return EXIT_FAILURE;
  const auto& compile_command = result.Ok();

  std::ofstream output_stream{
      absl::GetFlag(FLAGS_output_path),
      std::ios::out | std::ios::trunc | std::ios::binary};
  const auto success = compile_command.SerializeToOstream(&output_stream);
  if (!success) return EXIT_FAILURE;

  google::protobuf::ShutdownProtobufLibrary();

  return EXIT_SUCCESS;
}
