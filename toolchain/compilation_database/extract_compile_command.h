#ifndef SPOOR_TOOLCHAIN_COMPILATION_DATABASE_EXTRACT_COMPILE_COMMAND_H_
#define SPOOR_TOOLCHAIN_COMPILATION_DATABASE_EXTRACT_COMPILE_COMMAND_H_

#include <gflags/gflags.h>

#include <string>

#include "util/flags.h"

namespace toolchain::compilation_database {

const char* const kExtractCompileCommandVersion = "1.0.0";
const char* const kUsage =
    "Extract the command to compile each C++ source file from its `CppCompile` "
    "build mnemonic.";

DEFINE_string(  // NOLINT
    extra_action_file, "",
    "Path to an extra action file provided by the Bazel action listener.");

DEFINE_validator(  // NOLINT
    extra_action_file, &util::flags::ValidateInputFilePath);

DEFINE_string(  // NOLINT
    output, "", "Path where the generated compile command should be saved.");

DEFINE_validator(  // NOLINT
    output, &util::flags::ValidateOutputFilePath);

}  // namespace toolchain::compilation_database

#endif
