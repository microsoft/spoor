#include "toolchain/compilation_database/compilation_database_util.h"

#include <filesystem>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/strings/str_join.h"
#include "external/com_bazelbuild_bazel/src/main/protobuf/extra_actions_base.pb.h"
#include "google/protobuf/stubs/common.h"
#include "google/protobuf/stubs/status.h"
#include "google/protobuf/util/json_util.h"
#include "gsl/gsl"
#include "toolchain/compilation_database/compile_commands.pb.h"
#include "util/result.h"

namespace toolchain::compilation_database {

using util::result::None;

auto ParseExtraActionInfo(gsl::not_null<std::istream*> input_stream)
    -> util::result::Result<CompileCommand, ParseExtraActionInfoError> {
  blaze::ExtraActionInfo extra_action_info{};
  const auto success = extra_action_info.ParseFromIstream(input_stream);
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
  arguments.insert(std::end(arguments),
                   std::cbegin(compile_info.compiler_option()),
                   std::cend(compile_info.compiler_option()));

  const auto command = absl::StrJoin(arguments, " ");
  compile_command.set_command(command);

  return compile_command;
}

auto SerializeCompileCommandToOutputStream(
    const CompileCommand& compile_command,
    gsl::not_null<std::ostream*> output_stream)
    -> util::result::Result<None, None> {
  using Result = util::result::Result<None, None>;
  const auto success = compile_command.SerializeToOstream(output_stream);
  return success ? Result::Ok({}) : Result::Err({});
}

auto ConcatenateCompileCommands(
    const std::vector<std::filesystem::path>& individual_compile_command_files,
    const std::filesystem::path& compile_command_directory,
    const std::function<std::variant<std::ifstream, std::istringstream>(
        const std::filesystem::path&)>
        make_input_stream)
    -> util::result::Result<CompileCommands, ConcatenateCompileCommandsError> {
  CompileCommands compile_commands{};
  for (const auto& input_file : individual_compile_command_files) {
    auto* compile_command = compile_commands.add_compile_commands();
    auto input_stream = make_input_stream(input_file);
    const auto success = std::visit(
        [&compile_command](auto& input_stream) {
          return compile_command->ParseFromIstream(&input_stream);
        },
        input_stream);
    if (!success) return ConcatenateCompileCommandsError::kParsingError;
    compile_command->set_directory(compile_command_directory);
  }
  return compile_commands;
}

auto SerializeCompileCommandsToOutputStream(
    const CompileCommands& compile_commands,
    gsl::not_null<std::ostream*> output_stream)
    -> util::result::Result<None, google::protobuf::util::Status> {
  using Result = util::result::Result<None, google::protobuf::util::Status>;
  std::string json{};
  google::protobuf::util::JsonPrintOptions options{};
  const auto status = google::protobuf::util::MessageToJsonString(
      compile_commands, &json, options);
  if (!status.ok()) return status;

  // Hack: Isolate the array from the protobuf's JSON output to produce the
  // correct `compile_commands.json` format.
  std::ostream_iterator<char> output_iterator{*output_stream};
  const auto begin = std::cbegin(json) + json.find('[');
  const auto end = std::cbegin(json) + json.rfind(']') + 1;
  std::copy(begin, end, output_iterator);

  return Result::Ok({});
}

}  // namespace toolchain::compilation_database
