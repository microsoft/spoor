// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "toolchain/compilation_database/compilation_database_util.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "external/com_bazelbuild_bazel/src/main/protobuf/extra_actions_base.pb.h"
#include "google/protobuf/util/message_differencer.h"
#include "gtest/gtest.h"
#include "toolchain/compilation_database/compile_commands.pb.h"

namespace {

using google::protobuf::util::MessageDifferencer;
using toolchain::compilation_database::CompileCommand;
using toolchain::compilation_database::CompileCommands;
using toolchain::compilation_database::ConcatenateCompileCommands;
using toolchain::compilation_database::ConcatenateCompileCommandsError;
using toolchain::compilation_database::ParseExtraActionInfo;
using toolchain::compilation_database::ParseExtraActionInfoError;
using toolchain::compilation_database::SerializeCompileCommandsToOutputStream;
using toolchain::compilation_database::SerializeCompileCommandToOutputStream;

TEST(ParseExtraActionInfo, ParsesFromInputStream) {  // NOLINT
  const std::string compile_tool{"c++"};
  const std::string source_file{"source_file.cc"};
  const std::vector<std::string> compile_options{"--foo", "bar", "--baz"};
  const blaze::ExtraActionInfo extra_action_info = [&]() {
    blaze::ExtraActionInfo extra_action_info{};
    auto* cpp_compile_info = extra_action_info.MutableExtension(
        blaze::CppCompileInfo::cpp_compile_info);
    cpp_compile_info->set_tool(compile_tool);
    cpp_compile_info->set_source_file(source_file);
    for (const auto& option : compile_options) {
      auto* compiler_option = cpp_compile_info->add_compiler_option();
      *compiler_option = option;
    }
    return extra_action_info;
  }();
  const CompileCommand expected_compile_command = [&]() {
    CompileCommand compile_command{};
    compile_command.set_file(source_file);
    const auto command =
        absl::StrCat(compile_tool, " ", absl::StrJoin(compile_options, " "));
    compile_command.set_command(command);
    return compile_command;
  }();

  std::stringstream buffer{};
  const auto success = extra_action_info.SerializeToOstream(&buffer);
  ASSERT_TRUE(success);

  const auto result = ParseExtraActionInfo(&buffer);
  ASSERT_TRUE(result.IsOk());
  const auto& compile_command = result.Ok();
  ASSERT_TRUE(
      MessageDifferencer::Equals(compile_command, expected_compile_command));
}

TEST(ParseExtraActionInfo, FailsParsesFromInputStream) {  // NOLINT
  std::istringstream buffer{"bad data"};
  const auto result = ParseExtraActionInfo(&buffer);
  ASSERT_TRUE(result.IsErr());
  ASSERT_EQ(result.Err(), ParseExtraActionInfoError::kParsingError);
}

TEST(ParseExtraActionInfo, FailsIfMissingExtension) {  // NOLINT
  const blaze::ExtraActionInfo extra_action_info{};

  std::stringstream buffer{};
  const auto success = extra_action_info.SerializeToOstream(&buffer);
  ASSERT_TRUE(success);

  const auto result = ParseExtraActionInfo(&buffer);
  ASSERT_TRUE(result.IsErr());
  ASSERT_EQ(result.Err(),
            ParseExtraActionInfoError::kExtraActionInfoMissingExtension);
}

// NOLINTNEXTLINE
TEST(SerializeCompileCommandToOutputStream, SerializesCompileCommand) {
  const CompileCommand compile_command = [&]() {
    CompileCommand compile_command{};
    compile_command.set_file("source_file.cc");
    compile_command.set_command("c++ --foo bar --baz");
    return compile_command;
  }();

  std::stringstream expected_buffer{};
  const auto success = compile_command.SerializeToOstream(&expected_buffer);
  ASSERT_TRUE(success);

  std::stringstream buffer{};
  const auto result =
      SerializeCompileCommandToOutputStream(compile_command, &buffer);
  ASSERT_TRUE(result.IsOk());
  ASSERT_EQ(buffer.str(), expected_buffer.str());
}

// NOLINTNEXTLINE
TEST(SerializeCompileCommandToOutputStream, FailsToSerializesCompileCommand) {
  const CompileCommand compile_command = [&]() {
    CompileCommand compile_command{};
    compile_command.set_file("source_file.cc");
    compile_command.set_command("c++ --foo bar --baz");
    return compile_command;
  }();

  std::ofstream bad_output_stream{
      "", std::ios::out | std::ios::trunc | std::ios::binary};
  const auto result = SerializeCompileCommandToOutputStream(compile_command,
                                                            &bad_output_stream);
  ASSERT_TRUE(result.IsErr());
}

TEST(ConcatenateCompileCommands, ConcatenatesCompileCommands) {  // NOLINT
  const std::string compile_command_directory{"/path"};
  const std::vector<std::filesystem::path> compile_command_files{
      "a.compile_command.pb", "b.compile_command.pb"};
  const CompileCommand compile_command_a = [&]() {
    CompileCommand compile_command{};
    compile_command.set_directory(compile_command_directory);
    compile_command.set_file("source_file_a.cc");
    compile_command.set_command("c++ --foo bar --baz source_file_a.cc");
    return compile_command;
  }();
  const CompileCommand compile_command_b = [&]() {
    CompileCommand compile_command{};
    compile_command.set_directory(compile_command_directory);
    compile_command.set_file("source_file_b.cc");
    compile_command.set_command("c++ --foo bar --baz source_file_b.cc");
    return compile_command;
  }();
  const CompileCommands expected_compile_commands = [&]() {
    CompileCommands compile_commands{};
    *compile_commands.add_compile_commands() = compile_command_a;
    *compile_commands.add_compile_commands() = compile_command_b;
    return compile_commands;
  }();
  const auto make_input_stream = [&](const std::filesystem::path& input_file)
      -> std::variant<std::ifstream, std::istringstream> {
    std::ostringstream buffer{};
    if (input_file == "a.compile_command.pb") {
      compile_command_a.SerializeToOstream(&buffer);
    }
    if (input_file == "b.compile_command.pb") {
      compile_command_b.SerializeToOstream(&buffer);
    }
    return std::istringstream{buffer.str()};
  };
  const auto result = ConcatenateCompileCommands(
      compile_command_files, compile_command_directory, make_input_stream);
  ASSERT_TRUE(result.IsOk());
  const auto& compile_commands = result.Ok();
  ASSERT_TRUE(
      MessageDifferencer::Equals(compile_commands, expected_compile_commands));
}

TEST(ConcatenateCompileCommands, FailsToParseInput) {  // NOLINT
  const std::vector<std::filesystem::path> compile_command_files{
      "compile_command_a.pb", "compile_command_b.pb"};
  const auto make_input_stream =
      [&](const std::filesystem::path&
          /*unused*/) -> std::variant<std::ifstream, std::istringstream> {
    return std::istringstream{"bad data"};
  };
  const auto result = ConcatenateCompileCommands(compile_command_files, "/path",
                                                 make_input_stream);
  ASSERT_TRUE(result.IsErr());
  ASSERT_EQ(result.Err(), ConcatenateCompileCommandsError::kParsingError);
}

// NOLINTNEXTLINE
TEST(SerializeCompileCommandsToOutputStream, SerializesCompileCommands) {
  const CompileCommands compile_commands = []() {
    CompileCommand compile_command{};
    compile_command.set_directory("d");
    compile_command.set_file("f");
    compile_command.set_command("c");
    CompileCommands compile_commands{};
    *compile_commands.add_compile_commands() = compile_command;
    return compile_commands;
  }();
  const std::string expected_json{
      R"([{"directory":"d","file":"f","command":"c"}])"};

  std::ostringstream buffer{};
  const auto result =
      SerializeCompileCommandsToOutputStream(compile_commands, &buffer);
  ASSERT_TRUE(result.IsOk());
  ASSERT_EQ(buffer.str(), expected_json);
}

}  // namespace
