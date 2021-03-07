// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/inject_runtime/inject_runtime.h"

#include <cstdlib>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <unordered_set>
#include <vector>

#include "gmock/gmock.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/util/time_util.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "spoor/proto/spoor.pb.h"
#include "util/numeric.h"
#include "util/time/clock.h"
#include "util/time/clock_mock.h"

namespace {

using google::protobuf::util::MessageDifferencer;
using google::protobuf::util::TimeUtil;
using spoor::FunctionInfo;
using spoor::InstrumentedFunctionMap;
using spoor::instrumentation::inject_runtime::InjectRuntime;
using spoor::instrumentation::inject_runtime::
    kInstrumentedFunctionMapFileExtension;
using testing::MatchesRegex;
using testing::Return;
using util::time::SystemClock;
using util::time::testing::MakeTimePoint;
using util::time::testing::SystemClockMock;

constexpr std::string_view kUninstrumentedIrFile{
    "spoor/instrumentation/test_data/fib.ll"};
constexpr std::string_view kUninstrumentedIrWithDebugInfoCSourceFile{
    "spoor/instrumentation/test_data/fib_debug_c.ll"};
constexpr std::string_view kUninstrumentedIrWithDebugInfoCppSourceFile{
    "spoor/instrumentation/test_data/fib_debug_cpp.ll"};
constexpr std::string_view kUninstrumentedIrWithDebugInfoObjcSourceFile{
    "spoor/instrumentation/test_data/fib_debug_objc.ll"};
constexpr std::string_view kUninstrumentedIrWithDebugInfoSwiftSourceFile{
    "spoor/instrumentation/test_data/fib_debug_swift.ll"};
constexpr std::string_view kInstrumentedIrFile{
    "spoor/instrumentation/test_data/fib_instrumented.ll"};
constexpr std::string_view kInstrumentedInitializedIrFile{
    "spoor/instrumentation/test_data/fib_instrumented_initialized.ll"};
constexpr std::string_view kInstrumentedInitializedEnabledIrFile{
    "spoor/instrumentation/test_data/fib_instrumented_initialized_enabled.ll"};
constexpr std::string_view kNoMainIrFile{
    "spoor/instrumentation/test_data/fib_no_main.ll"};
constexpr std::string_view kOnlyMainFunctionInstrumentedIrFile{
    "spoor/instrumentation/test_data/fib_only_main_instrumented.ll"};

auto AssertModulesEqual(gsl::not_null<llvm::Module*> computed_module,
                        gsl::not_null<llvm::Module*> expected_module) -> void {
  // Hack: Modules are equal if their IR string representations are equal,
  // except for the module identifier and source file name. A nice side effect
  // of this approach is that we get a diff in the error message.
  const std::string computed_module_id{computed_module->getModuleIdentifier()};
  computed_module->setModuleIdentifier({});
  const std::string computed_module_source_file{
      computed_module->getSourceFileName()};
  computed_module->setSourceFileName({});
  const std::string expected_module_id{computed_module->getModuleIdentifier()};
  expected_module->setModuleIdentifier({});
  const std::string expected_module_source_file{
      expected_module->getSourceFileName()};
  expected_module->setSourceFileName({});

  std::string computed_module_ir{};
  llvm::raw_string_ostream{computed_module_ir} << *computed_module;

  std::string expected_module_ir{};
  llvm::raw_string_ostream{expected_module_ir} << *expected_module;

  ASSERT_EQ(expected_module_ir, computed_module_ir);

  computed_module->setModuleIdentifier(computed_module_id);
  computed_module->setSourceFileName(computed_module_source_file);
  expected_module->setModuleIdentifier(expected_module_id);
  expected_module->setSourceFileName(expected_module_source_file);
}

TEST(InjectRuntime, InstrumentsModule) {  // NOLINT
  struct TestCaseConfig {
    std::string_view expected_ir_file;
    bool initialize_runtime;
    bool enable_runtime;
  };
  const std::vector<TestCaseConfig> configs{
      {kInstrumentedIrFile, false, false},
      {kInstrumentedIrFile, false, true},
      {kInstrumentedInitializedIrFile, true, false},
      {kInstrumentedInitializedEnabledIrFile, true, true}};

  for (const auto& config : configs) {
    llvm::SMDiagnostic instrumented_module_diagnostic{};
    llvm::LLVMContext instrumented_module_context{};
    auto expected_instrumented_module = llvm::parseIRFile(
        config.expected_ir_file.data(), instrumented_module_diagnostic,
        instrumented_module_context);
    ASSERT_NE(expected_instrumented_module, nullptr);

    llvm::SMDiagnostic uninstrumented_module_diagnostic{};
    llvm::LLVMContext uninstrumented_module_context{};
    auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                           uninstrumented_module_diagnostic,
                                           uninstrumented_module_context);
    ASSERT_NE(parsed_module, nullptr);

    const auto ostream = [](const llvm::StringRef /*unused*/,
                            gsl::not_null<std::error_code*> /*unused*/) {
      return std::make_unique<llvm::raw_null_ostream>();
    };
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectRuntime inject_runtime{
        {.instrumented_function_map_output_path = "/",
         .instrumented_function_map_output_stream = ostream,
         .system_clock = std::move(system_clock),
         .function_allow_list = {},
         .function_blocklist = {},
         .module_id = {},
         .min_instruction_count_to_instrument = 0,
         .initialize_runtime = config.initialize_runtime,
         .enable_runtime = config.enable_runtime}};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_runtime.run(*parsed_module, module_analysis_manager);
    {
      SCOPED_TRACE("Modules are not equal.");
      AssertModulesEqual(parsed_module.get(),
                         expected_instrumented_module.get());
    }
  }
}

TEST(InjectRuntime, OutputsInstrumentedFunctionMap) {  // NOLINT
  std::vector<std::pair<std::string_view, InstrumentedFunctionMap>> test_cases{
      {
          kUninstrumentedIrFile,
          [&] {
            InstrumentedFunctionMap expected_instrumented_function_map{};
            expected_instrumented_function_map.set_module_id(
                kUninstrumentedIrFile.data());
            *expected_instrumented_function_map.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(0);
            auto& function_map =
                *expected_instrumented_function_map.mutable_function_map();

            FunctionInfo fibonacci_function_info{};
            fibonacci_function_info.set_linkage_name("_Z9Fibonaccii");
            fibonacci_function_info.set_demangled_name("Fibonacci(int)");
            fibonacci_function_info.set_instrumented(true);
            function_map[0x73c43c2b00000000] = fibonacci_function_info;

            FunctionInfo main_function_info{};
            main_function_info.set_linkage_name("main");
            main_function_info.set_demangled_name("main");
            main_function_info.set_instrumented(true);
            function_map[0x73c43c2b00000001] = main_function_info;

            return expected_instrumented_function_map;
          }(),
      },
      {
          kUninstrumentedIrWithDebugInfoCSourceFile,
          [&] {
            InstrumentedFunctionMap expected_instrumented_function_map{};
            expected_instrumented_function_map.set_module_id(
                kUninstrumentedIrWithDebugInfoCSourceFile.data());
            *expected_instrumented_function_map.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(0);
            auto& function_map =
                *expected_instrumented_function_map.mutable_function_map();

            FunctionInfo fibonacci_function_info{};
            fibonacci_function_info.set_linkage_name("Fibonacci");
            fibonacci_function_info.set_demangled_name("Fibonacci");
            fibonacci_function_info.set_file_name("fibonacci.c");
            fibonacci_function_info.set_directory("/path/to/file");
            fibonacci_function_info.set_line(1);
            fibonacci_function_info.set_instrumented(true);
            function_map[0x753f031f00000000] = fibonacci_function_info;

            FunctionInfo main_function_info{};
            main_function_info.set_linkage_name("main");
            main_function_info.set_demangled_name("main");
            main_function_info.set_file_name("fibonacci.c");
            main_function_info.set_directory("/path/to/file");
            main_function_info.set_line(6);
            main_function_info.set_instrumented(true);
            function_map[0x753f031f00000001] = main_function_info;

            return expected_instrumented_function_map;
          }(),
      },
      {
          kUninstrumentedIrWithDebugInfoCppSourceFile,
          [&] {
            InstrumentedFunctionMap expected_instrumented_function_map{};
            expected_instrumented_function_map.set_module_id(
                kUninstrumentedIrWithDebugInfoCppSourceFile.data());
            *expected_instrumented_function_map.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(0);
            auto& function_map =
                *expected_instrumented_function_map.mutable_function_map();

            FunctionInfo fibonacci_function_info{};
            fibonacci_function_info.set_linkage_name("_Z9Fibonaccii");
            fibonacci_function_info.set_demangled_name("Fibonacci(int)");
            fibonacci_function_info.set_file_name("fibonacci.cc");
            fibonacci_function_info.set_directory("/path/to/file");
            fibonacci_function_info.set_line(1);
            fibonacci_function_info.set_instrumented(true);
            function_map[0xc373298f00000000] = fibonacci_function_info;

            FunctionInfo main_function_info{};
            main_function_info.set_linkage_name("main");
            main_function_info.set_demangled_name("main");
            main_function_info.set_file_name("fibonacci.cc");
            main_function_info.set_directory("/path/to/file");
            main_function_info.set_line(6);
            main_function_info.set_instrumented(true);
            function_map[0xc373298f00000001] = main_function_info;

            return expected_instrumented_function_map;
          }(),
      },
      {
          kUninstrumentedIrWithDebugInfoObjcSourceFile,
          [&] {
            InstrumentedFunctionMap expected_instrumented_function_map{};
            expected_instrumented_function_map.set_module_id(
                kUninstrumentedIrWithDebugInfoObjcSourceFile.data());
            *expected_instrumented_function_map.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(0);
            auto& function_map =
                *expected_instrumented_function_map.mutable_function_map();

            FunctionInfo fibonacci_function_info{};
            fibonacci_function_info.set_linkage_name(
                "\001+[Fibonacci compute:]");
            fibonacci_function_info.set_demangled_name("+[Fibonacci compute:]");
            fibonacci_function_info.set_file_name("fibonacci.m");
            fibonacci_function_info.set_directory("/path/to/file");
            fibonacci_function_info.set_line(6);
            fibonacci_function_info.set_instrumented(true);
            function_map[0x0917362100000000] = fibonacci_function_info;

            FunctionInfo main_function_info{};
            main_function_info.set_linkage_name("main");
            main_function_info.set_demangled_name("main");
            main_function_info.set_file_name("fibonacci.m");
            main_function_info.set_directory("/path/to/file");
            main_function_info.set_line(12);
            main_function_info.set_instrumented(true);
            function_map[0x0917362100000001] = main_function_info;

            return expected_instrumented_function_map;
          }(),
      },
      {
          kUninstrumentedIrWithDebugInfoSwiftSourceFile,
          [&] {
            InstrumentedFunctionMap expected_instrumented_function_map{};
            expected_instrumented_function_map.set_module_id(
                kUninstrumentedIrWithDebugInfoSwiftSourceFile.data());
            *expected_instrumented_function_map.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(0);
            auto& function_map =
                *expected_instrumented_function_map.mutable_function_map();

            FunctionInfo fibonacci_function_info{};
            fibonacci_function_info.set_linkage_name("$s9fibonacciAAyS2iF");
            fibonacci_function_info.set_demangled_name(
                "fibonacci.fibonacci(Swift.Int) -> Swift.Int");
            fibonacci_function_info.set_file_name("fibonacci.swift");
            fibonacci_function_info.set_directory("/path/to/file");
            fibonacci_function_info.set_line(1);
            fibonacci_function_info.set_instrumented(true);
            function_map[0x88a7c6bf00000001] = fibonacci_function_info;

            FunctionInfo main_function_info{};
            main_function_info.set_linkage_name("main");
            main_function_info.set_demangled_name("main");
            main_function_info.set_file_name("fibonacci.swift");
            main_function_info.set_directory("/path/to/file");
            // Swift automatically adds a `main` function and picks the line
            // number.
            main_function_info.set_line(1);
            main_function_info.set_instrumented(true);
            function_map[0x88a7c6bf00000000] = main_function_info;

            return expected_instrumented_function_map;
          }(),
      },
  };
  for (const auto& [ir_file, expected_instrumented_function_map] : test_cases) {
    llvm::SMDiagnostic instrumented_module_diagnostic{};
    llvm::LLVMContext instrumented_module_context{};
    auto parsed_module =
        llvm::parseIRFile(ir_file.data(), instrumented_module_diagnostic,
                          instrumented_module_context);
    ASSERT_NE(parsed_module, nullptr);

    std::string buffer{};
    const auto ostream = [&buffer](const llvm::StringRef /*unused*/,
                                   gsl::not_null<std::error_code*> /*unused*/) {
      return std::make_unique<llvm::raw_string_ostream>(buffer);
    };
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectRuntime inject_runtime{
        {.instrumented_function_map_output_path = "/",
         .instrumented_function_map_output_stream = ostream,
         .system_clock = std::move(system_clock),
         .function_allow_list = {},
         .function_blocklist = {},
         .module_id = {},
         .min_instruction_count_to_instrument = 0,
         .initialize_runtime = false,
         .enable_runtime = false}};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_runtime.run(*parsed_module, module_analysis_manager);

    InstrumentedFunctionMap instrumented_function_map{};
    instrumented_function_map.ParseFromString(buffer);

    ASSERT_TRUE(MessageDifferencer::Equals(instrumented_function_map,
                                           expected_instrumented_function_map));
  }
}

TEST(InjectRuntime, FunctionBlocklist) {  // NOLINT
  llvm::SMDiagnostic expected_module_diagnostic{};
  llvm::LLVMContext expected_module_context{};
  auto expected_instrumented_module =
      llvm::parseIRFile(kOnlyMainFunctionInstrumentedIrFile.data(),
                        expected_module_diagnostic, expected_module_context);
  ASSERT_NE(expected_instrumented_module, nullptr);

  llvm::SMDiagnostic uninstrumented_module_diagnostic{};
  llvm::LLVMContext uninstrumented_module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         uninstrumented_module_diagnostic,
                                         uninstrumented_module_context);
  ASSERT_NE(parsed_module, nullptr);

  const auto ostream = [](const llvm::StringRef /*unused*/,
                          gsl::not_null<std::error_code*> /*unused*/) {
    return std::make_unique<llvm::raw_null_ostream>();
  };
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  InjectRuntime inject_runtime{
      {.instrumented_function_map_output_path = "/",
       .instrumented_function_map_output_stream = ostream,
       .system_clock = std::move(system_clock),
       .function_allow_list = {},
       .function_blocklist = {"_Z9Fibonaccii"},
       .module_id = {},
       .min_instruction_count_to_instrument = 0,
       .initialize_runtime = false,
       .enable_runtime = false}};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_runtime.run(*parsed_module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(parsed_module.get(), expected_instrumented_module.get());
  }
}

TEST(InjectRuntime, FunctionAllowListOverridesBlocklist) {  // NOLINT
  llvm::SMDiagnostic expected_module_diagnostic{};
  llvm::LLVMContext expected_module_context{};
  auto expected_instrumented_module =
      llvm::parseIRFile(kInstrumentedIrFile.data(), expected_module_diagnostic,
                        expected_module_context);
  ASSERT_NE(expected_instrumented_module, nullptr);

  llvm::SMDiagnostic uninstrumented_module_diagnostic{};
  llvm::LLVMContext uninstrumented_module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         uninstrumented_module_diagnostic,
                                         uninstrumented_module_context);
  ASSERT_NE(parsed_module, nullptr);

  auto ostream = [](const llvm::StringRef /*unused*/,
                    gsl::not_null<std::error_code*> /*unused*/) {
    return std::make_unique<llvm::raw_null_ostream>();
  };
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  InjectRuntime inject_runtime{
      {.instrumented_function_map_output_path = "/",
       .instrumented_function_map_output_stream = ostream,
       .system_clock = std::move(system_clock),
       .function_allow_list = {"_Z9Fibonaccii"},
       .function_blocklist = {"_Z9Fibonaccii"},
       .module_id = {},
       .min_instruction_count_to_instrument = 0,
       .initialize_runtime = false,
       .enable_runtime = false}};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_runtime.run(*parsed_module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(parsed_module.get(), expected_instrumented_module.get());
  }
}

TEST(InjectRuntime, InstructionThreshold) {  // NOLINT
  struct TestCaseConfig {
    std::string_view expected_ir_file;
    uint32 instruction_threshold;
  };
  const std::vector<TestCaseConfig> configs{
      {kInstrumentedIrFile, 0},
      {kInstrumentedIrFile, 8},
      {kInstrumentedIrFile, 9},
      {kOnlyMainFunctionInstrumentedIrFile, 10},
      {kOnlyMainFunctionInstrumentedIrFile, 11}};
  for (const auto& config : configs) {
    llvm::SMDiagnostic expected_module_diagnostic{};
    llvm::LLVMContext expected_module_context{};
    auto expected_instrumented_module =
        llvm::parseIRFile(config.expected_ir_file.data(),
                          expected_module_diagnostic, expected_module_context);
    ASSERT_NE(expected_instrumented_module, nullptr);

    llvm::SMDiagnostic uninstrumented_module_diagnostic{};
    llvm::LLVMContext uninstrumented_module_context{};
    auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                           uninstrumented_module_diagnostic,
                                           uninstrumented_module_context);
    ASSERT_NE(parsed_module, nullptr);

    const auto ostream = [](const llvm::StringRef /*unused*/,
                            gsl::not_null<std::error_code*> /*unused*/) {
      return std::make_unique<llvm::raw_null_ostream>();
    };
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectRuntime inject_runtime{
        {.instrumented_function_map_output_path = "/",
         .instrumented_function_map_output_stream = ostream,
         .system_clock = std::move(system_clock),
         .function_allow_list = {},
         .function_blocklist = {},
         .module_id = {},
         .min_instruction_count_to_instrument = config.instruction_threshold,
         .initialize_runtime = false,
         .enable_runtime = false}};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_runtime.run(*parsed_module, module_analysis_manager);
    {
      SCOPED_TRACE("Modules are not equal.");
      AssertModulesEqual(parsed_module.get(),
                         expected_instrumented_module.get());
    }
  }
}

TEST(InjectRuntime, AlwaysInstrumentsMain) {  // NOLINT
  llvm::SMDiagnostic expected_module_diagnostic{};
  llvm::LLVMContext expected_module_context{};
  auto expected_instrumented_module =
      llvm::parseIRFile(kOnlyMainFunctionInstrumentedIrFile.data(),
                        expected_module_diagnostic, expected_module_context);
  ASSERT_NE(expected_instrumented_module, nullptr);

  llvm::SMDiagnostic uninstrumented_module_diagnostic{};
  llvm::LLVMContext uninstrumented_module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         uninstrumented_module_diagnostic,
                                         uninstrumented_module_context);
  ASSERT_NE(parsed_module, nullptr);

  const auto ostream = [](const llvm::StringRef /*unused*/,
                          gsl::not_null<std::error_code*> /*unused*/) {
    return std::make_unique<llvm::raw_null_ostream>();
  };
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  InjectRuntime inject_runtime{
      {.instrumented_function_map_output_path = "/",
       .instrumented_function_map_output_stream = ostream,
       .system_clock = std::move(system_clock),
       .function_allow_list = {},
       .function_blocklist = {"main", "_Z9Fibonaccii"},
       .module_id = {},
       .min_instruction_count_to_instrument =
           std::numeric_limits<uint32>::max(),
       .initialize_runtime = false,
       .enable_runtime = false}};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_runtime.run(*parsed_module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(parsed_module.get(), expected_instrumented_module.get());
  }
}

TEST(InjectRuntime, InstrumentedFunctionMapFileName) {  // NOLINT
  llvm::SMDiagnostic expected_module_diagnostic{};
  llvm::LLVMContext expected_module_context{};
  auto expected_instrumented_module =
      llvm::parseIRFile(kOnlyMainFunctionInstrumentedIrFile.data(),
                        expected_module_diagnostic, expected_module_context);
  ASSERT_NE(expected_instrumented_module, nullptr);

  llvm::SMDiagnostic uninstrumented_module_diagnostic{};
  llvm::LLVMContext uninstrumented_module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         uninstrumented_module_diagnostic,
                                         uninstrumented_module_context);
  ASSERT_NE(parsed_module, nullptr);
  constexpr std::string_view instrumented_function_map_output_path{
      "/path/to/output"};
  const std::vector<std::optional<std::string>> module_ids{{}, "", "module_id"};
  for (const auto& module_id : module_ids) {
    std::string file_name{};
    const auto ostream = [&file_name](
                             const llvm::StringRef file,
                             gsl::not_null<std::error_code*> /*unused*/) {
      file_name = file;
      return std::make_unique<llvm::raw_null_ostream>();
    };
    const auto expected_file_name = [&] {
      std::string buffer{};
      const auto id = module_id.value_or(parsed_module->getModuleIdentifier());
      const auto module_id_hash = std::hash<std::string>{}(id);
      llvm::raw_string_ostream{buffer}
          << instrumented_function_map_output_path << '/'
          << llvm::format_hex_no_prefix(module_id_hash,
                                        sizeof(module_id_hash) * 2)
          << '.' << kInstrumentedFunctionMapFileExtension;
      return buffer;
    }();
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectRuntime inject_runtime{
        {.instrumented_function_map_output_path =
             instrumented_function_map_output_path,
         .instrumented_function_map_output_stream = ostream,
         .system_clock = std::move(system_clock),
         .function_allow_list = {},
         .function_blocklist = {},
         .module_id = module_id,
         .min_instruction_count_to_instrument = 0,
         .initialize_runtime = false,
         .enable_runtime = false}};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_runtime.run(*parsed_module, module_analysis_manager);
    const std::string pattern{
        R"(/path/to/output/[a-f0-9]{16}\.spoor_function_map)"};
    ASSERT_THAT(file_name, MatchesRegex(pattern));
    ASSERT_EQ(file_name, expected_file_name);
  }
}

TEST(InjectRuntime, AddsTimestamp) {  // NOLINT
  llvm::SMDiagnostic module_diagnostic{};
  llvm::LLVMContext module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         module_diagnostic, module_context);
  ASSERT_NE(parsed_module, nullptr);

  std::string buffer{};
  const auto ostream = [&buffer](const llvm::StringRef /*unused*/,
                                 gsl::not_null<std::error_code*> /*unused*/) {
    return std::make_unique<llvm::raw_string_ostream>(buffer);
  };
  const auto nanoseconds = 1'607'590'800'000'000'000;
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(nanoseconds)));
  InjectRuntime inject_runtime{
      {.instrumented_function_map_output_path = "/path/to/output",
       .instrumented_function_map_output_stream = ostream,
       .system_clock = std::move(system_clock),
       .function_allow_list = {},
       .function_blocklist = {},
       .min_instruction_count_to_instrument = 0,
       .initialize_runtime = false,
       .enable_runtime = false}};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_runtime.run(*parsed_module, module_analysis_manager);

  InstrumentedFunctionMap instrumented_function_map{};
  instrumented_function_map.ParseFromString(buffer);

  ASSERT_EQ(
      TimeUtil::TimestampToNanoseconds(instrumented_function_map.created_at()),
      nanoseconds);
}

TEST(InjectRuntime, ReturnValue) {  // NOLINT
  struct TestCaseConfig {
    std::unordered_set<std::string> function_blocklist;
    bool are_all_preserved;
  };
  const std::vector<TestCaseConfig> configs{{{}, false},
                                            {{"_Z9Fibonaccii"}, true}};

  for (const auto& config : configs) {
    llvm::SMDiagnostic module_diagnostic{};
    llvm::LLVMContext module_context{};
    auto parsed_module = llvm::parseIRFile(kNoMainIrFile.data(),
                                           module_diagnostic, module_context);
    ASSERT_NE(parsed_module, nullptr);

    const std::string path{"/"};
    const auto ostream = [](const llvm::StringRef /*unused*/,
                            gsl::not_null<std::error_code*> /*unused*/) {
      return std::make_unique<llvm::raw_null_ostream>();
    };
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectRuntime inject_runtime{
        {.instrumented_function_map_output_path = "/",
         .instrumented_function_map_output_stream = ostream,
         .system_clock = std::move(system_clock),
         .function_allow_list = {},
         .function_blocklist = config.function_blocklist,
         .module_id = {},
         .min_instruction_count_to_instrument = 0,
         .initialize_runtime = false,
         .enable_runtime = false}};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    const auto result =
        inject_runtime.run(*parsed_module, module_analysis_manager);
    ASSERT_EQ(result.areAllPreserved(), config.are_all_preserved);
  }
}

TEST(InjectRuntime, ExitsOnOstreamError) {  // NOLINT
  llvm::SMDiagnostic expected_module_diagnostic{};
  llvm::LLVMContext expected_module_context{};
  auto expected_instrumented_module =
      llvm::parseIRFile(kOnlyMainFunctionInstrumentedIrFile.data(),
                        expected_module_diagnostic, expected_module_context);
  ASSERT_NE(expected_instrumented_module, nullptr);

  llvm::SMDiagnostic uninstrumented_module_diagnostic{};
  llvm::LLVMContext uninstrumented_module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         uninstrumented_module_diagnostic,
                                         uninstrumented_module_context);
  ASSERT_NE(parsed_module, nullptr);

  const std::string path{"/path/to/file"};
  auto error = std::make_error_code(std::errc::permission_denied);
  const auto ostream = [&error](const llvm::StringRef /*unused*/,
                                gsl::not_null<std::error_code*> error_code) {
    *error_code = error;
    return std::make_unique<llvm::raw_null_ostream>();
  };
  auto system_clock = std::make_unique<SystemClock>();
  InjectRuntime inject_runtime{
      {.instrumented_function_map_output_path = "/",
       .instrumented_function_map_output_stream = ostream,
       .system_clock = std::move(system_clock),
       .function_allow_list = {},
       .function_blocklist = {},
       .module_id = {},
       .min_instruction_count_to_instrument = 0,
       .initialize_runtime = false,
       .enable_runtime = false}};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  const std::string pattern{
      "error: Failed to open/create the instrumentation map output file "
      "'.*'\\. Permission denied\\..*"};
  ASSERT_EXIT(  // NOLINT
      inject_runtime.run(*parsed_module, module_analysis_manager),
      [](const int exit_code) { return exit_code != EXIT_SUCCESS; },
      MatchesRegex(pattern));
}

}  // namespace
