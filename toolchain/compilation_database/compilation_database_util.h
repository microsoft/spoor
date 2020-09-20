#ifndef SPOOR_TOOLCHAIN_COMPILATION_DATABASE_COMPILATION_DATABASE_UTIL_H_
#define SPOOR_TOOLCHAIN_COMPILATION_DATABASE_COMPILATION_DATABASE_UTIL_H_

#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

#include "google/protobuf/stubs/status.h"
#include "gsl/gsl"
#include "toolchain/compilation_database/compile_commands.pb.h"
#include "util/result.h"

namespace toolchain::compilation_database {

enum class ParseExtraActionInfoError {
  kParsingError,
  kExtraActionInfoMissingExtension,
};

enum class ConcatenateCompileCommandsError {
  kParsingError,
};

auto ParseExtraActionInfo(gsl::not_null<std::istream*> input_stream)
    -> util::result::Result<CompileCommand, ParseExtraActionInfoError>;
auto SerializeCompileCommandToOutputStream(
    const CompileCommand& compile_command,
    gsl::not_null<std::ostream*> output_stream)
    -> util::result::Result<util::result::None, util::result::None>;
auto ConcatenateCompileCommands(
    const std::vector<std::string>& individual_compile_command_files,
    const std::string& compile_command_directory,
    std::function<
        std::variant<std::ifstream, std::istringstream>(const std::string_view)>
        make_input_stream)
    -> util::result::Result<CompileCommands, ConcatenateCompileCommandsError>;
auto SerializeCompileCommandsToOutputStream(
    const CompileCommands& compile_commands,
    gsl::not_null<std::ostream*> output_stream)
    -> util::result::Result<util::result::None, google::protobuf::util::Status>;

}  // namespace toolchain::compilation_database

#endif
