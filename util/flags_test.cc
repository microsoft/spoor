#include "util/flags.h"

#include "gtest/gtest.h"
#include "util/mock/ifstream.h"

namespace {

using util::flags::AbslParseFlag;
using util::flags::AbslUnparseFlag;
using util::flags::InputFilePath;
using util::flags::OutputPath;

TEST(AbslParseFlagInputFilePath, SetsTypeOnSuccess) {  // NOLINT
  const std::string path{"path"};
  InputFilePath input_file_path{};
  std::string error{};
  const auto success =
      AbslParseFlag<util::mock::Ifstream<true>>(path, &input_file_path, &error);
  ASSERT_TRUE(success);
  ASSERT_EQ(input_file_path.input_file_path, path);
  ASSERT_EQ(error, "");
}

TEST(AbslUnparseFlagInputFilePath, ReturnsErrorMessageOnFailure) {  // NOLINT
  const std::string path{"path"};
  InputFilePath input_file_path{};
  std::string error{};
  const auto success = AbslParseFlag<util::mock::Ifstream<false>>(
      path, &input_file_path, &error);
  ASSERT_FALSE(success);
  ASSERT_EQ(input_file_path.input_file_path, path);
  ASSERT_EQ(error, "The input file `path` does not exist.");
}

TEST(AbslParseFlagOutputPath, SetsTypeOnSuccess) {  // NOLINT
  const std::string path{"/foo/bar"};
  OutputPath output_path{};
  std::string error{};
  const auto success = AbslParseFlag(path, &output_path, &error);
  ASSERT_TRUE(success);
  ASSERT_EQ(output_path.output_path, path);
  ASSERT_EQ(error, "");
}

TEST(AbslUnparseFlagOutputPath, ReturnsErrorMessageOnFailure) {  // NOLINT
  const std::string path{};
  OutputPath output_path{};
  std::string error{};
  const auto success = AbslParseFlag(path, &output_path, &error);
  ASSERT_FALSE(success);
  ASSERT_EQ(output_path.output_path, path);
  ASSERT_EQ(error, "The output path must not be empty.");
}

}  // namespace
