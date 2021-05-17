// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/inject_instrumentation/inject_instrumentation.h"

#include <array>
#include <cstdlib>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "city_hash/city.h"
#include "gmock/gmock.h"
#include "google/protobuf/util/message_differencer.h"
#include "google/protobuf/util/time_util.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "spoor/proto/spoor.pb.h"
#include "util/numeric.h"
#include "util/time/clock.h"
#include "util/time/clock_mock.h"

namespace {

using google::protobuf::util::MessageDifferencer;
using google::protobuf::util::TimeUtil;
using spoor::FunctionInfo;
using spoor::InstrumentedFunctionMap;
using spoor::instrumentation::inject_instrumentation::InjectInstrumentation;
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

TEST(InjectInstrumentation, InstrumentsModule) {  // NOLINT
  struct alignas(32) TestCaseConfig {
    std::string_view expected_ir_file;
    bool initialize_runtime;
    bool enable_runtime;
  };
  constexpr std::array<TestCaseConfig, 4> configs{
      {{kInstrumentedIrFile, false, false},
       {kInstrumentedIrFile, false, true},
       {kInstrumentedInitializedIrFile, true, false},
       {kInstrumentedInitializedEnabledIrFile, true, true}}};

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

    auto output_function_map_stream = std::make_unique<std::ostringstream>();
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectInstrumentation inject_instrumentation{
        {.inject_instrumentation = true,
         .output_function_map_stream = std::move(output_function_map_stream),
         .system_clock = std::move(system_clock),
         .function_allow_list = {},
         .function_blocklist = {},
         .module_id = {},
         .min_instruction_count_to_instrument = 0,
         .initialize_runtime = config.initialize_runtime,
         .enable_runtime = config.enable_runtime}};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);
    {
      SCOPED_TRACE("Modules are not equal.");
      AssertModulesEqual(parsed_module.get(),
                         expected_instrumented_module.get());
    }
  }
}

TEST(InjectInstrumentation, OutputsInstrumentedFunctionMap) {  // NOLINT
  const auto make_module_id_hash = [](const std::string_view& module_id) {
    return static_cast<uint64>(CityHash32(module_id.data(), module_id.size()))
           << 32ULL;
  };
  const std::vector<std::pair<std::string_view, InstrumentedFunctionMap>>
      test_cases{
          {
              kUninstrumentedIrFile,
              [&] {
                constexpr auto module_id{kUninstrumentedIrFile};
                const auto module_id_hash = make_module_id_hash(module_id);

                InstrumentedFunctionMap expected_instrumented_function_map{};
                expected_instrumented_function_map.set_module_id(
                    module_id.data());
                *expected_instrumented_function_map.mutable_created_at() =
                    TimeUtil::NanosecondsToTimestamp(0);
                auto& function_map =
                    *expected_instrumented_function_map.mutable_function_map();

                FunctionInfo fibonacci_function_info{};
                fibonacci_function_info.set_linkage_name("_Z9Fibonaccii");
                fibonacci_function_info.set_demangled_name("Fibonacci(int)");
                fibonacci_function_info.set_instrumented(true);
                function_map[module_id_hash | 0ULL] = fibonacci_function_info;

                FunctionInfo main_function_info{};
                main_function_info.set_linkage_name("main");
                main_function_info.set_demangled_name("main");
                main_function_info.set_instrumented(true);
                function_map[module_id_hash | 1ULL] = main_function_info;

                return expected_instrumented_function_map;
              }(),
          },
          {
              kUninstrumentedIrWithDebugInfoCSourceFile,
              [&] {
                constexpr auto module_id{
                    kUninstrumentedIrWithDebugInfoCSourceFile};
                const auto module_id_hash = make_module_id_hash(module_id);

                InstrumentedFunctionMap expected_instrumented_function_map{};
                expected_instrumented_function_map.set_module_id(
                    module_id.data());
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
                function_map[module_id_hash | 0ULL] = fibonacci_function_info;

                FunctionInfo main_function_info{};
                main_function_info.set_linkage_name("main");
                main_function_info.set_demangled_name("main");
                main_function_info.set_file_name("fibonacci.c");
                main_function_info.set_directory("/path/to/file");
                main_function_info.set_line(6);
                main_function_info.set_instrumented(true);
                function_map[module_id_hash | 1ULL] = main_function_info;

                return expected_instrumented_function_map;
              }(),
          },
          {
              kUninstrumentedIrWithDebugInfoCppSourceFile,
              [&] {
                constexpr auto module_id{
                    kUninstrumentedIrWithDebugInfoCppSourceFile};
                const auto module_id_hash = make_module_id_hash(module_id);

                InstrumentedFunctionMap expected_instrumented_function_map{};
                expected_instrumented_function_map.set_module_id(
                    module_id.data());
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
                function_map[module_id_hash | 0ULL] = fibonacci_function_info;

                FunctionInfo main_function_info{};
                main_function_info.set_linkage_name("main");
                main_function_info.set_demangled_name("main");
                main_function_info.set_file_name("fibonacci.cc");
                main_function_info.set_directory("/path/to/file");
                main_function_info.set_line(6);
                main_function_info.set_instrumented(true);
                function_map[module_id_hash | 1ULL] = main_function_info;

                return expected_instrumented_function_map;
              }(),
          },
          {
              kUninstrumentedIrWithDebugInfoObjcSourceFile,
              [&] {
                constexpr auto module_id{
                    kUninstrumentedIrWithDebugInfoObjcSourceFile};
                const auto module_id_hash = make_module_id_hash(module_id);

                InstrumentedFunctionMap expected_instrumented_function_map{};
                expected_instrumented_function_map.set_module_id(
                    module_id.data());
                *expected_instrumented_function_map.mutable_created_at() =
                    TimeUtil::NanosecondsToTimestamp(0);
                auto& function_map =
                    *expected_instrumented_function_map.mutable_function_map();

                FunctionInfo fibonacci_function_info{};
                fibonacci_function_info.set_linkage_name(
                    "\001+[Fibonacci compute:]");
                fibonacci_function_info.set_demangled_name(
                    "+[Fibonacci compute:]");
                fibonacci_function_info.set_file_name("fibonacci.m");
                fibonacci_function_info.set_directory("/path/to/file");
                fibonacci_function_info.set_line(6);
                fibonacci_function_info.set_instrumented(true);
                function_map[module_id_hash | 0ULL] = fibonacci_function_info;

                FunctionInfo main_function_info{};
                main_function_info.set_linkage_name("main");
                main_function_info.set_demangled_name("main");
                main_function_info.set_file_name("fibonacci.m");
                main_function_info.set_directory("/path/to/file");
                main_function_info.set_line(12);
                main_function_info.set_instrumented(true);
                function_map[module_id_hash | 1ULL] = main_function_info;

                return expected_instrumented_function_map;
              }(),
          },
          {
              kUninstrumentedIrWithDebugInfoSwiftSourceFile,
              [&] {
                constexpr auto module_id{
                    kUninstrumentedIrWithDebugInfoSwiftSourceFile};
                const auto module_id_hash = make_module_id_hash(module_id);

                InstrumentedFunctionMap expected_instrumented_function_map{};
                expected_instrumented_function_map.set_module_id(
                    module_id.data());
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
                function_map[module_id_hash | 1ULL] = fibonacci_function_info;

                FunctionInfo main_function_info{};
                main_function_info.set_linkage_name("main");
                main_function_info.set_demangled_name("main");
                main_function_info.set_file_name("fibonacci.swift");
                main_function_info.set_directory("/path/to/file");
                // Swift automatically adds a `main` function and picks the line
                // number.
                main_function_info.set_line(1);
                main_function_info.set_instrumented(true);
                function_map[module_id_hash | 0ULL] = main_function_info;

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

    auto output_function_map_stream = std::make_unique<std::ostringstream>();
    auto* output_function_map_buffer = output_function_map_stream->rdbuf();
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectInstrumentation inject_instrumentation{
        {.inject_instrumentation = true,
         .output_function_map_stream = std::move(output_function_map_stream),
         .system_clock = std::move(system_clock),
         .function_allow_list = {},
         .function_blocklist = {},
         .module_id = {},
         .min_instruction_count_to_instrument = 0,
         .initialize_runtime = false,
         .enable_runtime = false}};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);

    InstrumentedFunctionMap instrumented_function_map{};
    instrumented_function_map.ParseFromString(
        output_function_map_buffer->str());

    ASSERT_TRUE(MessageDifferencer::Equals(instrumented_function_map,
                                           expected_instrumented_function_map));
  }
}

TEST(InjectInstrumentation, FunctionBlocklist) {  // NOLINT
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

  auto output_function_map_stream = std::make_unique<std::ostringstream>();
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  InjectInstrumentation inject_instrumentation{
      {.inject_instrumentation = true,
       .output_function_map_stream = std::move(output_function_map_stream),
       .system_clock = std::move(system_clock),
       .function_allow_list = {},
       .function_blocklist = {"_Z9Fibonaccii"},
       .module_id = {},
       .min_instruction_count_to_instrument = 0,
       .initialize_runtime = false,
       .enable_runtime = false}};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_instrumentation.run(*parsed_module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(parsed_module.get(), expected_instrumented_module.get());
  }
}

TEST(InjectInstrumentation, FunctionAllowListOverridesBlocklist) {  // NOLINT
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

  auto output_function_map_stream = std::make_unique<std::ostringstream>();
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  InjectInstrumentation inject_instrumentation{
      {.inject_instrumentation = true,
       .output_function_map_stream = std::move(output_function_map_stream),
       .system_clock = std::move(system_clock),
       .function_allow_list = {"_Z9Fibonaccii"},
       .function_blocklist = {"_Z9Fibonaccii"},
       .module_id = {},
       .min_instruction_count_to_instrument = 0,
       .initialize_runtime = false,
       .enable_runtime = false}};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_instrumentation.run(*parsed_module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(parsed_module.get(), expected_instrumented_module.get());
  }
}

TEST(InjectInstrumentation, InstructionThreshold) {  // NOLINT
  struct alignas(32) TestCaseConfig {
    std::string_view expected_ir_file;
    uint32 instruction_threshold;
  };
  constexpr std::array<TestCaseConfig, 5> configs{
      {{kInstrumentedIrFile, 0},
       {kInstrumentedIrFile, 8},
       {kInstrumentedIrFile, 9},
       {kOnlyMainFunctionInstrumentedIrFile, 10},
       {kOnlyMainFunctionInstrumentedIrFile, 11}}};
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

    auto output_function_map_stream = std::make_unique<std::ostringstream>();
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectInstrumentation inject_instrumentation{
        {.inject_instrumentation = true,
         .output_function_map_stream = std::move(output_function_map_stream),
         .system_clock = std::move(system_clock),
         .function_allow_list = {},
         .function_blocklist = {},
         .module_id = {},
         .min_instruction_count_to_instrument = config.instruction_threshold,
         .initialize_runtime = false,
         .enable_runtime = false}};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);
    {
      SCOPED_TRACE("Modules are not equal.");
      AssertModulesEqual(parsed_module.get(),
                         expected_instrumented_module.get());
    }
  }
}

TEST(InjectInstrumentation, AlwaysInstrumentsMain) {  // NOLINT
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

  auto output_function_map_stream = std::make_unique<std::ostringstream>();
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  InjectInstrumentation inject_instrumentation{
      {.inject_instrumentation = true,
       .output_function_map_stream = std::move(output_function_map_stream),
       .system_clock = std::move(system_clock),
       .function_allow_list = {},
       .function_blocklist = {"main", "_Z9Fibonaccii"},
       .module_id = {},
       .min_instruction_count_to_instrument =
           std::numeric_limits<uint32>::max(),
       .initialize_runtime = false,
       .enable_runtime = false}};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_instrumentation.run(*parsed_module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(parsed_module.get(), expected_instrumented_module.get());
  }
}

TEST(InjectInstrumentation, DoNotInjectInstrumentationConfig) {  // NOLINT
  llvm::SMDiagnostic expected_module_diagnostic{};
  llvm::LLVMContext expected_module_context{};
  auto expected_uninstrumented_module =
      llvm::parseIRFile(kUninstrumentedIrFile, expected_module_diagnostic,
                        expected_module_context);
  ASSERT_NE(expected_uninstrumented_module, nullptr);

  llvm::SMDiagnostic uninstrumented_module_diagnostic{};
  llvm::LLVMContext uninstrumented_module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         uninstrumented_module_diagnostic,
                                         uninstrumented_module_context);
  ASSERT_NE(parsed_module, nullptr);

  auto output_function_map_stream = std::make_unique<std::ostringstream>();
  auto system_clock = std::make_unique<SystemClockMock>();
  InjectInstrumentation inject_instrumentation{
      {.inject_instrumentation = false,
       .output_function_map_stream = std::move(output_function_map_stream),
       .system_clock = std::move(system_clock),
       .function_allow_list = {},
       .function_blocklist = {},
       .module_id = {},
       .min_instruction_count_to_instrument =
           std::numeric_limits<uint32>::max(),
       .initialize_runtime = false,
       .enable_runtime = false}};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_instrumentation.run(*parsed_module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(parsed_module.get(),
                       expected_uninstrumented_module.get());
  }
}

TEST(InjectInstrumentation, AddsTimestamp) {  // NOLINT
  llvm::SMDiagnostic module_diagnostic{};
  llvm::LLVMContext module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         module_diagnostic, module_context);
  ASSERT_NE(parsed_module, nullptr);

  auto output_function_map_stream = std::make_unique<std::ostringstream>();
  auto* output_function_map_buffer = output_function_map_stream->rdbuf();
  const auto nanoseconds = 1'607'590'800'000'000'000;
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(nanoseconds)));
  InjectInstrumentation inject_instrumentation{
      {.inject_instrumentation = true,
       .output_function_map_stream = std::move(output_function_map_stream),
       .system_clock = std::move(system_clock),
       .function_allow_list = {},
       .function_blocklist = {},
       .min_instruction_count_to_instrument = 0,
       .initialize_runtime = false,
       .enable_runtime = false}};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_instrumentation.run(*parsed_module, module_analysis_manager);

  InstrumentedFunctionMap instrumented_function_map{};
  instrumented_function_map.ParseFromString(output_function_map_buffer->str());

  ASSERT_EQ(
      TimeUtil::TimestampToNanoseconds(instrumented_function_map.created_at()),
      nanoseconds);
}

TEST(InjectInstrumentation, ReturnValue) {  // NOLINT
  struct alignas(64) TestCaseConfig {
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

    auto output_function_map_stream = std::make_unique<std::ostringstream>();
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectInstrumentation inject_instrumentation{
        {.inject_instrumentation = true,
         .output_function_map_stream = std::move(output_function_map_stream),
         .system_clock = std::move(system_clock),
         .function_allow_list = {},
         .function_blocklist = config.function_blocklist,
         .module_id = {},
         .min_instruction_count_to_instrument = 0,
         .initialize_runtime = false,
         .enable_runtime = false}};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    const auto result =
        inject_instrumentation.run(*parsed_module, module_analysis_manager);
    ASSERT_EQ(result.areAllPreserved(), config.are_all_preserved);
  }
}

}  // namespace
