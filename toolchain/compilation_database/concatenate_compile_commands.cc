#include "toolchain/compilation_database/concatenate_compile_commands.h"

#include <gflags/gflags.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/util/json_util.h>

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "toolchain/compilation_database/compile_commands.pb.h"
#include "util/result.h"

namespace {

using toolchain::compilation_database::CompileCommands;

enum class ConcatenateCompileCommandsError {
  kParsingError,
};

auto ConcatenateCompileCommands(
    const std::vector<std::string>& individual_compile_command_files,
    const std::string& compile_command_directory)
    -> util::result::Result<CompileCommands, ConcatenateCompileCommandsError> {
  CompileCommands compile_commands{};
  for (const auto& input_file : individual_compile_command_files) {
    auto* compile_command = compile_commands.add_compile_commands();
    std::ifstream input_stream{input_file, std::ios::in | std::ios::binary};
    const auto success = compile_command->ParseFromIstream(&input_stream);
    if (!success) return ConcatenateCompileCommandsError::kParsingError;
    compile_command->set_directory(compile_command_directory);
  }
  return compile_commands;
}

}  // namespace

auto main(int argc, char** argv) -> int {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  gflags::SetVersionString(
      toolchain::compilation_database::kConcatenateCompileCommandsVersion);
  gflags::SetUsageMessage(toolchain::compilation_database::kUsage);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::vector<std::string> input_files{};
  std::copy(std::istream_iterator<std::string>(std::cin),
            std::istream_iterator<std::string>(),
            std::back_inserter(input_files));

  const auto compile_commands = ConcatenateCompileCommands(
      input_files,
      toolchain::compilation_database::FLAGS_compile_command_directory);
  if (!compile_commands.IsOk()) return EXIT_FAILURE;

  std::string json{};
  google::protobuf::util::JsonPrintOptions options{};
  const auto status = google::protobuf::util::MessageToJsonString(
      compile_commands.Ok(), &json, options);
  if (!status.ok()) return EXIT_FAILURE;

  // Hack: Isolate the array from the protobuf's JSON output to produce the
  // correct `compile_commands.json` format.
  std::ofstream output_stream{
      toolchain::compilation_database::FLAGS_output_compilation_database,
      std::ios::out | std::ios::trunc | std::ios::binary};
  std::ostream_iterator<char> output_iterator{output_stream};
  const auto begin = json.begin() + json.find('[');
  const auto end = json.begin() + json.rfind(']') + 1;
  std::copy(begin, end, output_iterator);

  google::protobuf::ShutdownProtobufLibrary();

  return EXIT_SUCCESS;
}
