// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/file_source.h"

#include <algorithm>
#include <filesystem>
#include <iterator>
#include <memory>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "spoor/instrumentation/config/output_language.h"
#include "spoor/instrumentation/config/source.h"
#include "util/env/env.h"
#include "util/file_system/file_reader_mock.h"
#include "util/file_system/util.h"

namespace {

using spoor::instrumentation::config::FileSource;
using spoor::instrumentation::config::kEnableRuntimeFileKey;
using spoor::instrumentation::config::kFileConfigKeys;
using spoor::instrumentation::config::kFiltersFileFileKey;
using spoor::instrumentation::config::kForceBinaryOutputFileKey;
using spoor::instrumentation::config::kInitializeRuntimeFileKey;
using spoor::instrumentation::config::kInjectInstrumentationFileKey;
using spoor::instrumentation::config::kModuleIdFileKey;
using spoor::instrumentation::config::kOutputFileFileKey;
using spoor::instrumentation::config::kOutputLanguageFileKey;
using spoor::instrumentation::config::kOutputSymbolsFileFileKey;
using spoor::instrumentation::config::OutputLanguage;
using spoor::instrumentation::config::Source;
using testing::Return;
using testing::ReturnRef;
using util::env::kHomeKey;
using util::file_system::testing::FileReaderMock;

auto GetEmptyEnv(const char* /*unused*/) -> const char* { return nullptr; }

auto ErrorTypes(const std::vector<Source::ReadError>& errors)
    -> std::vector<Source::ReadError::Type> {
  std::vector<Source::ReadError::Type> error_types{};
  std::transform(std::cbegin(errors), std::cend(errors),
                 std::back_inserter(error_types),
                 [](const auto& error) { return error.type; });
  return error_types;
}

TEST(FileSource, InitialState) {  // NOLINT
  const FileSource file_source{{
      .file_reader = std::make_unique<FileReaderMock>(),
      .path_expansion_options{
          .get_env = GetEmptyEnv,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
  }};
  ASSERT_FALSE(file_source.IsRead());

  ASSERT_FALSE(file_source.GetEnableRuntime().has_value());
  ASSERT_FALSE(file_source.GetFiltersFile().has_value());
  ASSERT_FALSE(file_source.GetForceBinaryOutput().has_value());
  ASSERT_FALSE(file_source.GetInitializeRuntime().has_value());
  ASSERT_FALSE(file_source.GetInjectInstrumentation().has_value());
  ASSERT_FALSE(file_source.GetModuleId().has_value());
  ASSERT_FALSE(file_source.GetOutputFile().has_value());
  ASSERT_FALSE(file_source.GetOutputLanguage().has_value());
  ASSERT_FALSE(file_source.GetOutputSymbolsFile().has_value());
}

TEST(FileSource, ConfigsAreNulloptIfNotSet) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/config.toml"};
  std::stringstream empty_file_buffer{};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(empty_file_buffer));
  EXPECT_CALL(*file_reader, Close());

  FileSource file_source{{
      .file_reader = std::move(file_reader),
      .path_expansion_options{
          .get_env = GetEmptyEnv,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_path{file_path},
  }};

  const auto errors = file_source.Read();
  ASSERT_TRUE(file_source.IsRead());
  ASSERT_TRUE(errors.empty());

  ASSERT_FALSE(file_source.GetEnableRuntime().has_value());
  ASSERT_FALSE(file_source.GetFiltersFile().has_value());
  ASSERT_FALSE(file_source.GetForceBinaryOutput().has_value());
  ASSERT_FALSE(file_source.GetInitializeRuntime().has_value());
  ASSERT_FALSE(file_source.GetInjectInstrumentation().has_value());
  ASSERT_FALSE(file_source.GetModuleId().has_value());
  ASSERT_FALSE(file_source.GetOutputFile().has_value());
  ASSERT_FALSE(file_source.GetOutputLanguage().has_value());
  ASSERT_FALSE(file_source.GetOutputSymbolsFile().has_value());
}

TEST(FileSource, ParsesConfigs) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/config.toml"};

  std::stringstream buffer{};
  buffer << kEnableRuntimeFileKey << " = false\n"
         << kFiltersFileFileKey << " = '/path/to/filters.toml'\n"
         << kForceBinaryOutputFileKey << " = true\n"
         << kInitializeRuntimeFileKey << " = false\n"
         << kInjectInstrumentationFileKey << " = false\n"
         << kModuleIdFileKey << " = 'id'\n"
         << kOutputFileFileKey << " = '/path/to/output.ll'\n"
         << kOutputLanguageFileKey << " = 'ir'\n"
         << kOutputSymbolsFileFileKey << " = '/path/to/symbols.spoor_symbols'";

  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());

  FileSource file_source{{
      .file_reader{std::move(file_reader)},
      .path_expansion_options{
          .get_env = GetEmptyEnv,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_path{file_path},
  }};
  const auto errors = file_source.Read();
  ASSERT_TRUE(file_source.IsRead());
  ASSERT_TRUE(errors.empty());

  ASSERT_TRUE(file_source.GetEnableRuntime().has_value());
  ASSERT_FALSE(file_source.GetEnableRuntime().value());

  ASSERT_TRUE(file_source.GetFiltersFile().has_value());
  ASSERT_EQ(file_source.GetFiltersFile().value(), "/path/to/filters.toml");

  ASSERT_TRUE(file_source.GetForceBinaryOutput().has_value());
  ASSERT_TRUE(file_source.GetForceBinaryOutput().value());

  ASSERT_TRUE(file_source.GetInitializeRuntime().has_value());
  ASSERT_FALSE(file_source.GetInitializeRuntime().value());

  ASSERT_TRUE(file_source.GetInjectInstrumentation().has_value());
  ASSERT_FALSE(file_source.GetInjectInstrumentation().value());

  ASSERT_TRUE(file_source.GetModuleId().has_value());
  ASSERT_EQ(file_source.GetModuleId().value(), "id");

  ASSERT_TRUE(file_source.GetOutputFile().has_value());
  ASSERT_EQ(file_source.GetOutputFile().value(), "/path/to/output.ll");

  ASSERT_TRUE(file_source.GetOutputLanguage().has_value());
  ASSERT_EQ(file_source.GetOutputLanguage().value(), OutputLanguage::kIr);

  ASSERT_TRUE(file_source.GetOutputSymbolsFile().has_value());
  ASSERT_EQ(file_source.GetOutputSymbolsFile().value(),
            "/path/to/symbols.spoor_symbols");
}

TEST(FileSource, ExpandsPath) {  // NOLINT
  const auto get_env = [&](const char* key) -> const char* {
    constexpr util::flat_map::FlatMap<std::string_view, std::string_view, 2>
        environment{
            {kHomeKey, "/usr/you"},
            {"FOO", "bar"},
        };
    auto value = environment.FirstValueForKey(key);
    if (value.has_value()) return value.value().data();
    return nullptr;
  };

  const std::filesystem::path file_path{"/path/to/config.toml"};
  std::stringstream buffer{};
  buffer << kFiltersFileFileKey << " = '~/$FOO/filters.toml'\n"
         << kOutputFileFileKey << " = '~/$FOO/output.ll'\n"
         << kOutputSymbolsFileFileKey << " = '~/$FOO/symbols.spoor_symbols'";

  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());

  FileSource file_source{{
      .file_reader{std::move(file_reader)},
      .path_expansion_options{
          .get_env = get_env,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_path{file_path},
  }};
  const auto errors = file_source.Read();
  ASSERT_TRUE(file_source.IsRead());
  ASSERT_TRUE(errors.empty());

  ASSERT_FALSE(file_source.GetEnableRuntime().has_value());

  ASSERT_TRUE(file_source.GetFiltersFile().has_value());
  ASSERT_EQ(file_source.GetFiltersFile().value(), "/usr/you/bar/filters.toml");

  ASSERT_FALSE(file_source.GetForceBinaryOutput().has_value());

  ASSERT_FALSE(file_source.GetInitializeRuntime().has_value());

  ASSERT_FALSE(file_source.GetInjectInstrumentation().has_value());

  ASSERT_FALSE(file_source.GetModuleId().has_value());

  ASSERT_TRUE(file_source.GetOutputFile().has_value());
  ASSERT_EQ(file_source.GetOutputFile().value(), "/usr/you/bar/output.ll");

  ASSERT_FALSE(file_source.GetOutputLanguage().has_value());

  ASSERT_TRUE(file_source.GetOutputSymbolsFile().has_value());
  ASSERT_EQ(file_source.GetOutputSymbolsFile().value(),
            "/usr/you/bar/symbols.spoor_symbols");
}

TEST(FileSource, ConvertsRelativeToAbsolutePath) {  // NOLINT
  const std::filesystem::path file_path{"/usr/you/path/to/config.toml"};
  std::stringstream buffer{};
  buffer << kFiltersFileFileKey << " = 'subdir/filters.toml'\n"
         << kOutputFileFileKey << " = 'subdir/output.ll'\n"
         << kOutputSymbolsFileFileKey << " = 'subdir/symbols.spoor_symbols'";

  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());

  FileSource file_source{{
      .file_reader{std::move(file_reader)},
      .path_expansion_options{
          .get_env = GetEmptyEnv,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_path{file_path},
  }};
  const auto errors = file_source.Read();
  ASSERT_TRUE(file_source.IsRead());
  ASSERT_TRUE(errors.empty());

  ASSERT_FALSE(file_source.GetEnableRuntime().has_value());

  ASSERT_TRUE(file_source.GetFiltersFile().has_value());
  ASSERT_EQ(file_source.GetFiltersFile().value(),
            "/usr/you/path/to/subdir/filters.toml");

  ASSERT_FALSE(file_source.GetForceBinaryOutput().has_value());

  ASSERT_FALSE(file_source.GetInitializeRuntime().has_value());

  ASSERT_FALSE(file_source.GetInjectInstrumentation().has_value());

  ASSERT_FALSE(file_source.GetModuleId().has_value());

  ASSERT_TRUE(file_source.GetOutputFile().has_value());
  ASSERT_EQ(file_source.GetOutputFile().value(),
            "/usr/you/path/to/subdir/output.ll");

  ASSERT_FALSE(file_source.GetOutputLanguage().has_value());

  ASSERT_TRUE(file_source.GetOutputSymbolsFile().has_value());
  ASSERT_EQ(file_source.GetOutputSymbolsFile().value(),
            "/usr/you/path/to/subdir/symbols.spoor_symbols");
}

TEST(FileSource, NormalizesOutputLanguage) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/config.toml"};
  std::stringstream buffer{};
  buffer << kOutputLanguageFileKey << " = '   bItCoDe   '";

  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());

  FileSource file_source{{
      .file_reader{std::move(file_reader)},
      .path_expansion_options{
          .get_env = GetEmptyEnv,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_path{file_path},
  }};
  const auto errors = file_source.Read();
  ASSERT_TRUE(file_source.IsRead());
  ASSERT_TRUE(errors.empty());

  ASSERT_FALSE(file_source.GetEnableRuntime().has_value());

  ASSERT_FALSE(file_source.GetFiltersFile().has_value());

  ASSERT_FALSE(file_source.GetForceBinaryOutput().has_value());

  ASSERT_FALSE(file_source.GetInitializeRuntime().has_value());

  ASSERT_FALSE(file_source.GetInjectInstrumentation().has_value());

  ASSERT_FALSE(file_source.GetModuleId().has_value());

  ASSERT_FALSE(file_source.GetOutputFile().has_value());

  ASSERT_TRUE(file_source.GetOutputLanguage().has_value());
  ASSERT_EQ(file_source.GetOutputLanguage().value(), OutputLanguage::kBitcode);

  ASSERT_FALSE(file_source.GetOutputSymbolsFile().has_value());
}

TEST(FileSource, ReadsConfigOnce) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/config.toml"};
  std::stringstream empty_file_buffer{};

  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(empty_file_buffer));
  EXPECT_CALL(*file_reader, Close());

  FileSource file_source{{
      .file_reader{std::move(file_reader)},
      .path_expansion_options{
          .get_env = GetEmptyEnv,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_path{file_path},
  }};
  ASSERT_FALSE(file_source.IsRead());
  const auto errors = file_source.Read();
  ASSERT_TRUE(file_source.IsRead());
  ASSERT_TRUE(errors.empty());

  for (auto run{0}; run < 3; ++run) {
    const auto errors = file_source.Read();
    ASSERT_TRUE(file_source.IsRead());
    ASSERT_TRUE(errors.empty());
  }
}

TEST(FileSource, HandlesFailedToOpenFile) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/config.toml"};
  std::stringstream empty_file_buffer{};

  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(false));

  FileSource file_source{{
      .file_reader{std::move(file_reader)},
      .path_expansion_options{
          .get_env = GetEmptyEnv,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_path{file_path},
  }};
  ASSERT_FALSE(file_source.IsRead());
  auto errors = file_source.Read();

  const auto error_types = ErrorTypes(errors);
  const std::vector<Source::ReadError::Type> expected_error_types{
      Source::ReadError::Type::kFailedToOpenFile};
  ASSERT_EQ(error_types, expected_error_types);

  ASSERT_FALSE(file_source.GetEnableRuntime().has_value());
  ASSERT_FALSE(file_source.GetFiltersFile().has_value());
  ASSERT_FALSE(file_source.GetForceBinaryOutput().has_value());
  ASSERT_FALSE(file_source.GetInitializeRuntime().has_value());
  ASSERT_FALSE(file_source.GetInjectInstrumentation().has_value());
  ASSERT_FALSE(file_source.GetModuleId().has_value());
  ASSERT_FALSE(file_source.GetOutputFile().has_value());
  ASSERT_FALSE(file_source.GetOutputLanguage().has_value());
  ASSERT_FALSE(file_source.GetOutputSymbolsFile().has_value());

  errors = file_source.Read();
  ASSERT_TRUE(errors.empty());
}

TEST(FileSource, HandlesMalformedFile) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/config.toml"};

  std::stringstream buffer{"bad file"};

  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());

  FileSource file_source{{
      .file_reader{std::move(file_reader)},
      .path_expansion_options{
          .get_env = GetEmptyEnv,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_path{file_path},
  }};
  const auto errors = file_source.Read();
  ASSERT_TRUE(file_source.IsRead());

  const auto error_types = ErrorTypes(errors);
  const std::vector<Source::ReadError::Type> expected_error_types{
      Source::ReadError::Type::kMalformedFile};
  ASSERT_EQ(error_types, expected_error_types);

  ASSERT_FALSE(file_source.GetEnableRuntime().has_value());
  ASSERT_FALSE(file_source.GetFiltersFile().has_value());
  ASSERT_FALSE(file_source.GetForceBinaryOutput().has_value());
  ASSERT_FALSE(file_source.GetInitializeRuntime().has_value());
  ASSERT_FALSE(file_source.GetInjectInstrumentation().has_value());
  ASSERT_FALSE(file_source.GetModuleId().has_value());
  ASSERT_FALSE(file_source.GetOutputFile().has_value());
  ASSERT_FALSE(file_source.GetOutputLanguage().has_value());
  ASSERT_FALSE(file_source.GetOutputSymbolsFile().has_value());
}

TEST(FileSource, HandlesUnknownKey) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/config.toml"};

  std::stringstream buffer{};
  buffer << "unknown_key = false\n"
         << kEnableRuntimeFileKey << " = false\n"
         << kFiltersFileFileKey << " = '/path/to/filters.toml'\n"
         << kForceBinaryOutputFileKey << " = true\n"
         << kInitializeRuntimeFileKey << " = false\n"
         << kInjectInstrumentationFileKey << " = false\n"
         << kModuleIdFileKey << " = 'id'\n"
         << kOutputFileFileKey << " = '/path/to/output.ll'\n"
         << kOutputLanguageFileKey << " = 'ir'\n"
         << kOutputSymbolsFileFileKey << " = '/path/to/symbols.spoor_symbols'";

  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());

  FileSource file_source{{
      .file_reader{std::move(file_reader)},
      .path_expansion_options{
          .get_env = GetEmptyEnv,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_path{file_path},
  }};
  const auto errors = file_source.Read();
  ASSERT_TRUE(file_source.IsRead());

  const auto error_types = ErrorTypes(errors);
  const std::vector<Source::ReadError::Type> expected_error_types{
      Source::ReadError::Type::kUnknownKey};
  ASSERT_EQ(error_types, expected_error_types);

  ASSERT_TRUE(file_source.GetEnableRuntime().has_value());
  ASSERT_FALSE(file_source.GetEnableRuntime().value());

  ASSERT_TRUE(file_source.GetFiltersFile().has_value());
  ASSERT_EQ(file_source.GetFiltersFile().value(), "/path/to/filters.toml");

  ASSERT_TRUE(file_source.GetForceBinaryOutput().has_value());
  ASSERT_TRUE(file_source.GetForceBinaryOutput().value());

  ASSERT_TRUE(file_source.GetInitializeRuntime().has_value());
  ASSERT_FALSE(file_source.GetInitializeRuntime().value());

  ASSERT_TRUE(file_source.GetInjectInstrumentation().has_value());
  ASSERT_FALSE(file_source.GetInjectInstrumentation().value());

  ASSERT_TRUE(file_source.GetModuleId().has_value());
  ASSERT_EQ(file_source.GetModuleId().value(), "id");

  ASSERT_TRUE(file_source.GetOutputFile().has_value());
  ASSERT_EQ(file_source.GetOutputFile().value(), "/path/to/output.ll");

  ASSERT_TRUE(file_source.GetOutputLanguage().has_value());
  ASSERT_EQ(file_source.GetOutputLanguage().value(), OutputLanguage::kIr);

  ASSERT_TRUE(file_source.GetOutputSymbolsFile().has_value());
  ASSERT_EQ(file_source.GetOutputSymbolsFile().value(),
            "/path/to/symbols.spoor_symbols");
}

TEST(FileSource, HandlesBadValue) {  // NOLINT
  const std::filesystem::path file_path{"/path/to/config.toml"};

  std::stringstream buffer{};
  buffer << kEnableRuntimeFileKey << " = 'bad value'\n"
         << kFiltersFileFileKey << " = false\n"
         << kForceBinaryOutputFileKey << " = 'bad value'\n"
         << kInitializeRuntimeFileKey << " = 'bad value'\n"
         << kInjectInstrumentationFileKey << " = 'bad value'\n"
         << kModuleIdFileKey << " = false\n"
         << kOutputFileFileKey << " = false\n"
         << kOutputLanguageFileKey << " = 'bad value'\n"
         << kOutputSymbolsFileFileKey << " = false";

  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(buffer));
  EXPECT_CALL(*file_reader, Close());

  FileSource file_source{{
      .file_reader{std::move(file_reader)},
      .path_expansion_options{
          .get_env = GetEmptyEnv,
          .expand_tilde = true,
          .expand_environment_variables = true,
      },
      .file_path{file_path},
  }};
  const auto errors = file_source.Read();
  ASSERT_TRUE(file_source.IsRead());

  const auto error_types = ErrorTypes(errors);
  const auto expected_error_types = std::vector(
      kFileConfigKeys.size(), Source::ReadError::Type::kUnknownValue);
  ASSERT_EQ(error_types, expected_error_types);

  ASSERT_FALSE(file_source.GetEnableRuntime().has_value());
  ASSERT_FALSE(file_source.GetFiltersFile().has_value());
  ASSERT_FALSE(file_source.GetForceBinaryOutput().has_value());
  ASSERT_FALSE(file_source.GetInitializeRuntime().has_value());
  ASSERT_FALSE(file_source.GetInjectInstrumentation().has_value());
  ASSERT_FALSE(file_source.GetModuleId().has_value());
  ASSERT_FALSE(file_source.GetOutputFile().has_value());
  ASSERT_FALSE(file_source.GetOutputLanguage().has_value());
  ASSERT_FALSE(file_source.GetOutputSymbolsFile().has_value());
}

}  // namespace
