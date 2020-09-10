#include "toolchain/compilation_database/extract_compile_command.h"

#include <gflags/gflags.h>
#include <google/protobuf/stubs/common.h>

#include <cstdlib>
#include <fstream>
#include <numeric>
#include <string>
#include <tuple>
#include <vector>

#include "external/com_bazelbuild_bazel/src/main/protobuf/extra_actions_base.pb.h"
#include "toolchain/compilation_database/compile_commands.pb.h"
#include "util/result.h"
#include "util/strings.h"

namespace {

using toolchain::compilation_database::CompileCommand;

enum class ParseExtraActionInfoError {
  kParsingError,
  kExtraActionInfoMissingExtension,
};

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

  const auto command = util::strings::Join(arguments, " ");
  compile_command.set_command(command);

  return compile_command;
}

}  // namespace

auto main(int argc, char** argv) -> int {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  gflags::SetVersionString(
      toolchain::compilation_database::kExtractCompileCommandVersion);
  gflags::SetUsageMessage(toolchain::compilation_database::kUsage);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  const auto result = ParseExtraActionInfo(
      toolchain::compilation_database::FLAGS_extra_action_file);
  if (!result.IsOk()) return EXIT_FAILURE;
  const auto& compile_command = result.Ok();

  std::ofstream output_stream{
      toolchain::compilation_database::FLAGS_output,
      std::ios::out | std::ios::trunc | std::ios::binary};
  const auto success = compile_command.SerializeToOstream(&output_stream);
  if (!success) return EXIT_FAILURE;

  google::protobuf::ShutdownProtobufLibrary();

  return EXIT_SUCCESS;
}
