// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/inject_instrumentation/inject_instrumentation.h"

#include <array>
#include <cstdlib>
#include <ios>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "absl/strings/str_join.h"
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
#include "spoor/instrumentation/symbols/symbols.pb.h"
#include "spoor/instrumentation/symbols/symbols_writer.h"
#include "spoor/instrumentation/symbols/symbols_writer_mock.h"
#include "util/file_system/file_reader_mock.h"
#include "util/numeric.h"
#include "util/time/clock.h"
#include "util/time/clock_mock.h"

namespace {

using google::protobuf::util::MessageDifferencer;
using google::protobuf::util::TimeUtil;
using spoor::instrumentation::inject_instrumentation::InjectInstrumentation;
using spoor::instrumentation::symbols::Symbols;
using spoor::instrumentation::symbols::SymbolsWriter;
using spoor::instrumentation::symbols::testing::SymbolsWriterMock;
using testing::_;
using testing::Return;
using testing::ReturnRef;
using util::file_system::testing::FileReaderMock;
using util::time::SystemClock;
using util::time::testing::MakeTimePoint;
using util::time::testing::SystemClockMock;
using SymbolsWriterResult =
    spoor::instrumentation::symbols::testing::SymbolsWriterMock::Result;

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

MATCHER_P(SymbolsEq, expected, "Symbols are not equal.") {  // NOLINT
  return MessageDifferencer::Equals(arg, expected);
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
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

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

    auto file_reader = std::make_unique<FileReaderMock>();
    auto symbols_writer = std::make_unique<SymbolsWriterMock>();
    EXPECT_CALL(*symbols_writer, Write(symbols_file_path, _))
        .WillOnce(Return(SymbolsWriterResult::Ok({})));
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectInstrumentation inject_instrumentation{{
        .inject_instrumentation = true,
        .file_reader = std::move(file_reader),
        .symbols_writer = std::move(symbols_writer),
        .system_clock = std::move(system_clock),
        .function_allow_list_file_path = {},
        .function_blocklist_file_path = {},
        .symbols_file_path = symbols_file_path,
        .module_id = {},
        .min_instruction_count_to_instrument = 0,
        .initialize_runtime = config.initialize_runtime,
        .enable_runtime = config.enable_runtime,
    }};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);
    {
      SCOPED_TRACE("Modules are not equal.");
      AssertModulesEqual(parsed_module.get(),
                         expected_instrumented_module.get());
    }
  }
}

TEST(InjectInstrumentation, OutputsInstrumentedSymbols) {  // NOLINT
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};
  constexpr auto timestamp_0 = 1'607'590'800'000'000'000;
  constexpr auto timestamp_1 = 1'607'590'900'000'000'000;
  constexpr auto make_module_id_hash = [](const std::string_view module_id) {
    return static_cast<uint64>(CityHash32(module_id.data(), module_id.size()))
           << 32ULL;
  };
  const std::vector<std::pair<std::string_view, Symbols>> test_cases{
      {
          kUninstrumentedIrFile,
          [&] {
            Symbols symbols{};
            const std::string module_id{kUninstrumentedIrFile};
            const auto module_id_hash = make_module_id_hash(module_id);

            auto& function_symbols_table =
                *symbols.mutable_function_symbols_table();

            auto& fibonacci_function_infos =
                function_symbols_table[module_id_hash | 0ULL];
            auto& fibonacci_function_info =
                *fibonacci_function_infos.add_function_infos();
            fibonacci_function_info.set_module_id(module_id);
            fibonacci_function_info.set_linkage_name("_Z9Fibonaccii");
            fibonacci_function_info.set_demangled_name("Fibonacci(int)");
            fibonacci_function_info.set_instrumented(true);
            *fibonacci_function_info.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(timestamp_0);

            auto& main_function_infos =
                function_symbols_table[module_id_hash | 1ULL];
            auto& main_function_info =
                *main_function_infos.add_function_infos();
            main_function_info.set_module_id(module_id);
            main_function_info.set_linkage_name("main");
            main_function_info.set_demangled_name("main");
            main_function_info.set_instrumented(true);
            *main_function_info.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(timestamp_1);

            return symbols;
          }(),
      },
      {
          kUninstrumentedIrWithDebugInfoCSourceFile,
          [&] {
            Symbols symbols{};
            const std::string module_id{
                kUninstrumentedIrWithDebugInfoCSourceFile};
            const auto module_id_hash = make_module_id_hash(module_id);

            auto& function_symbols_table =
                *symbols.mutable_function_symbols_table();

            auto& fibonacci_function_infos =
                function_symbols_table[module_id_hash | 0ULL];
            auto& fibonacci_function_info =
                *fibonacci_function_infos.add_function_infos();
            fibonacci_function_info.set_module_id(module_id);
            fibonacci_function_info.set_linkage_name("Fibonacci");
            fibonacci_function_info.set_demangled_name("Fibonacci");
            fibonacci_function_info.set_file_name("fibonacci.c");
            fibonacci_function_info.set_directory("/path/to/file");
            fibonacci_function_info.set_line(1);
            fibonacci_function_info.set_instrumented(true);
            *fibonacci_function_info.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(timestamp_0);

            auto& main_function_infos =
                function_symbols_table[module_id_hash | 1ULL];
            auto& main_function_info =
                *main_function_infos.add_function_infos();
            main_function_info.set_module_id(module_id);
            main_function_info.set_linkage_name("main");
            main_function_info.set_demangled_name("main");
            main_function_info.set_file_name("fibonacci.c");
            main_function_info.set_directory("/path/to/file");
            main_function_info.set_line(6);
            main_function_info.set_instrumented(true);
            *main_function_info.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(timestamp_1);

            return symbols;
          }(),
      },
      {
          kUninstrumentedIrWithDebugInfoCppSourceFile,
          [&] {
            Symbols symbols{};
            const std::string module_id{
                kUninstrumentedIrWithDebugInfoCppSourceFile};
            const auto module_id_hash = make_module_id_hash(module_id);

            auto& function_symbols_table =
                *symbols.mutable_function_symbols_table();

            auto& fibonacci_function_infos =
                function_symbols_table[module_id_hash | 0ULL];
            auto& fibonacci_function_info =
                *fibonacci_function_infos.add_function_infos();
            fibonacci_function_info.set_module_id(module_id);
            fibonacci_function_info.set_linkage_name("_Z9Fibonaccii");
            fibonacci_function_info.set_demangled_name("Fibonacci(int)");
            fibonacci_function_info.set_file_name("fibonacci.cc");
            fibonacci_function_info.set_directory("/path/to/file");
            fibonacci_function_info.set_line(1);
            fibonacci_function_info.set_instrumented(true);
            *fibonacci_function_info.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(timestamp_0);

            auto& main_function_infos =
                function_symbols_table[module_id_hash | 1ULL];
            auto& main_function_info =
                *main_function_infos.add_function_infos();
            main_function_info.set_module_id(module_id);
            main_function_info.set_linkage_name("main");
            main_function_info.set_demangled_name("main");
            main_function_info.set_file_name("fibonacci.cc");
            main_function_info.set_directory("/path/to/file");
            main_function_info.set_line(6);
            fibonacci_function_info.set_instrumented(true);
            main_function_info.set_instrumented(true);
            *main_function_info.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(timestamp_1);

            return symbols;
          }(),
      },
      {
          kUninstrumentedIrWithDebugInfoObjcSourceFile,
          [&] {
            Symbols symbols{};
            const std::string module_id{
                kUninstrumentedIrWithDebugInfoObjcSourceFile};
            const auto module_id_hash = make_module_id_hash(module_id);

            auto& function_symbols_table =
                *symbols.mutable_function_symbols_table();

            auto& fibonacci_function_infos =
                function_symbols_table[module_id_hash | 0ULL];
            auto& fibonacci_function_info =
                *fibonacci_function_infos.add_function_infos();
            fibonacci_function_info.set_module_id(module_id);
            fibonacci_function_info.set_linkage_name(
                "\001+[Fibonacci compute:]");
            fibonacci_function_info.set_demangled_name("+[Fibonacci compute:]");
            fibonacci_function_info.set_file_name("fibonacci.m");
            fibonacci_function_info.set_directory("/path/to/file");
            fibonacci_function_info.set_line(6);
            fibonacci_function_info.set_instrumented(true);
            *fibonacci_function_info.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(timestamp_0);

            auto& main_function_infos =
                function_symbols_table[module_id_hash | 1ULL];
            auto& main_function_info =
                *main_function_infos.add_function_infos();
            main_function_info.set_module_id(module_id);
            main_function_info.set_linkage_name("main");
            main_function_info.set_demangled_name("main");
            main_function_info.set_file_name("fibonacci.m");
            main_function_info.set_directory("/path/to/file");
            main_function_info.set_line(12);
            main_function_info.set_instrumented(true);
            *main_function_info.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(timestamp_1);

            return symbols;
          }(),
      },
      {
          kUninstrumentedIrWithDebugInfoSwiftSourceFile,
          [&] {
            Symbols symbols{};
            const std::string module_id{
                kUninstrumentedIrWithDebugInfoSwiftSourceFile};
            const auto module_id_hash = make_module_id_hash(module_id);

            auto& function_symbols_table =
                *symbols.mutable_function_symbols_table();

            auto& main_function_infos =
                function_symbols_table[module_id_hash | 0ULL];
            auto& main_function_info =
                *main_function_infos.add_function_infos();
            main_function_info.set_module_id(module_id);
            main_function_info.set_linkage_name("main");
            main_function_info.set_demangled_name("main");
            main_function_info.set_file_name("fibonacci.swift");
            main_function_info.set_directory("/path/to/file");
            // Swift automatically adds a `main` function and picks the line
            // number.
            main_function_info.set_line(1);
            main_function_info.set_instrumented(true);
            *main_function_info.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(timestamp_0);

            auto& fibonacci_function_infos =
                function_symbols_table[module_id_hash | 1ULL];
            auto& fibonacci_function_info =
                *fibonacci_function_infos.add_function_infos();
            fibonacci_function_info.set_module_id(module_id);
            fibonacci_function_info.set_linkage_name("$s9fibonacciAAyS2iF");
            fibonacci_function_info.set_demangled_name(
                "fibonacci.fibonacci(Swift.Int) -> Swift.Int");
            fibonacci_function_info.set_file_name("fibonacci.swift");
            fibonacci_function_info.set_directory("/path/to/file");
            fibonacci_function_info.set_line(1);
            fibonacci_function_info.set_instrumented(true);
            *fibonacci_function_info.mutable_created_at() =
                TimeUtil::NanosecondsToTimestamp(timestamp_1);

            return symbols;
          }(),
      },
  };
  for (const auto& [ir_file, symbols] : test_cases) {
    llvm::SMDiagnostic instrumented_module_diagnostic{};
    llvm::LLVMContext instrumented_module_context{};
    auto parsed_module =
        llvm::parseIRFile(ir_file.data(), instrumented_module_diagnostic,
                          instrumented_module_context);
    ASSERT_NE(parsed_module, nullptr);

    auto file_reader = std::make_unique<FileReaderMock>();
    auto symbols_writer = std::make_unique<SymbolsWriterMock>();
    EXPECT_CALL(*symbols_writer, Write(symbols_file_path, SymbolsEq(symbols)))
        .WillOnce(Return(SymbolsWriterResult::Ok({})));
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillOnce(Return(MakeTimePoint<std::chrono::system_clock>(timestamp_0)))
        .WillOnce(
            Return(MakeTimePoint<std::chrono::system_clock>(timestamp_1)));
    InjectInstrumentation inject_instrumentation{{
        .inject_instrumentation = true,
        .file_reader = std::move(file_reader),
        .symbols_writer = std::move(symbols_writer),
        .system_clock = std::move(system_clock),
        .function_allow_list_file_path = {},
        .function_blocklist_file_path = {},
        .symbols_file_path = symbols_file_path,
        .module_id = {},
        .min_instruction_count_to_instrument = 0,
        .initialize_runtime = false,
        .enable_runtime = false,
    }};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);
  }
}

TEST(InjectInstrumentation, FunctionBlocklist) {  // NOLINT
  const std::filesystem::path blocklist_file_path{"/path/to/blocklist.txt"};
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

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

  std::stringstream blocklist{"_Z9Fibonaccii"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(blocklist_file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(blocklist));
  EXPECT_CALL(*file_reader, Close());
  auto symbols_writer = std::make_unique<SymbolsWriterMock>();
  EXPECT_CALL(*symbols_writer, Write(symbols_file_path, _))
      .WillOnce(Return(SymbolsWriterResult::Ok({})));
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  InjectInstrumentation inject_instrumentation{{
      .inject_instrumentation = true,
      .file_reader = std::move(file_reader),
      .symbols_writer = std::move(symbols_writer),
      .system_clock = std::move(system_clock),
      .function_allow_list_file_path = {},
      .function_blocklist_file_path = blocklist_file_path,
      .symbols_file_path = symbols_file_path,
      .module_id = {},
      .min_instruction_count_to_instrument = 0,
      .initialize_runtime = false,
      .enable_runtime = false,
  }};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_instrumentation.run(*parsed_module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(parsed_module.get(), expected_instrumented_module.get());
  }
}

TEST(InjectInstrumentation, FunctionAllowListOverridesBlocklist) {  // NOLINT
  const std::filesystem::path allow_list_file_path{"/path/to/allow_list.txt"};
  const std::filesystem::path blocklist_file_path{"/path/to/blocklist.txt"};
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

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

  std::stringstream allow_list{"_Z9Fibonaccii"};
  std::stringstream blocklist{"_Z9Fibonaccii"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(allow_list_file_path));
  EXPECT_CALL(*file_reader, Open(blocklist_file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillRepeatedly(Return(true));
  EXPECT_CALL(*file_reader, Istream())
      .WillOnce(ReturnRef(allow_list))
      .WillOnce(ReturnRef(blocklist));
  EXPECT_CALL(*file_reader, Close()).Times(2);
  auto symbols_writer = std::make_unique<SymbolsWriterMock>();
  EXPECT_CALL(*symbols_writer, Write(symbols_file_path, _))
      .WillOnce(Return(SymbolsWriterResult::Ok({})));
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  InjectInstrumentation inject_instrumentation{{
      .inject_instrumentation = true,
      .file_reader = std::move(file_reader),
      .symbols_writer = std::move(symbols_writer),
      .system_clock = std::move(system_clock),
      .function_allow_list_file_path = allow_list_file_path,
      .function_blocklist_file_path = blocklist_file_path,
      .symbols_file_path = symbols_file_path,
      .module_id = {},
      .min_instruction_count_to_instrument = 0,
      .initialize_runtime = false,
      .enable_runtime = false,
  }};
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
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

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

    auto file_reader = std::make_unique<FileReaderMock>();
    auto symbols_writer = std::make_unique<SymbolsWriterMock>();
    EXPECT_CALL(*symbols_writer, Write(symbols_file_path, _))
        .WillOnce(Return(SymbolsWriterResult::Ok({})));
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectInstrumentation inject_instrumentation{{
        .inject_instrumentation = true,
        .file_reader = std::move(file_reader),
        .symbols_writer = std::move(symbols_writer),
        .system_clock = std::move(system_clock),
        .function_allow_list_file_path = {},
        .function_blocklist_file_path = {},
        .symbols_file_path = symbols_file_path,
        .module_id = {},
        .min_instruction_count_to_instrument = config.instruction_threshold,
        .initialize_runtime = false,
        .enable_runtime = false,
    }};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);
    {
      SCOPED_TRACE("Modules are not equal.");
      AssertModulesEqual(parsed_module.get(),
                         expected_instrumented_module.get());
    }
  }
}

TEST(InjectInstrumentation, AllowListOverridesInstructionCount) {  // NOLINT
  const std::filesystem::path allow_list_file_path{"/path/to/allow_list.txt"};
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

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

  std::stringstream allow_list{"_Z9Fibonaccii"};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(allow_list_file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(allow_list));
  EXPECT_CALL(*file_reader, Close());
  auto symbols_writer = std::make_unique<SymbolsWriterMock>();
  EXPECT_CALL(*symbols_writer, Write(symbols_file_path, _))
      .WillOnce(Return(SymbolsWriterResult::Ok({})));
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  InjectInstrumentation inject_instrumentation{{
      .inject_instrumentation = true,
      .file_reader = std::move(file_reader),
      .symbols_writer = std::move(symbols_writer),
      .system_clock = std::move(system_clock),
      .function_allow_list_file_path = allow_list_file_path,
      .function_blocklist_file_path = {},
      .symbols_file_path = symbols_file_path,
      .module_id = {},
      .min_instruction_count_to_instrument = std::numeric_limits<uint32>::max(),
      .initialize_runtime = false,
      .enable_runtime = false,
  }};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_instrumentation.run(*parsed_module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(parsed_module.get(), expected_instrumented_module.get());
  }
}

TEST(InjectInstrumentation, AlwaysInstrumentsMain) {  // NOLINT
  const std::filesystem::path blocklist_file_path{"/path/to/blocklist.txt"};
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

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

  std::stringstream blocklist{absl::StrJoin({"main", "_Z9Fibonaccii"}, "\n")};
  auto file_reader = std::make_unique<FileReaderMock>();
  EXPECT_CALL(*file_reader, Open(blocklist_file_path));
  EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
  EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(blocklist));
  EXPECT_CALL(*file_reader, Close());
  auto symbols_writer = std::make_unique<SymbolsWriterMock>();
  EXPECT_CALL(*symbols_writer, Write(symbols_file_path, _))
      .WillOnce(Return(SymbolsWriterResult::Ok({})));
  auto system_clock = std::make_unique<SystemClockMock>();
  EXPECT_CALL(*system_clock, Now())
      .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
  InjectInstrumentation inject_instrumentation{{
      .inject_instrumentation = true,
      .file_reader = std::move(file_reader),
      .symbols_writer = std::move(symbols_writer),
      .system_clock = std::move(system_clock),
      .function_allow_list_file_path = {},
      .function_blocklist_file_path = blocklist_file_path,
      .symbols_file_path = symbols_file_path,
      .module_id = {},
      .min_instruction_count_to_instrument = std::numeric_limits<uint32>::max(),
      .initialize_runtime = false,
      .enable_runtime = false,
  }};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_instrumentation.run(*parsed_module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(parsed_module.get(), expected_instrumented_module.get());
  }
}

TEST(InjectInstrumentation, DoNotInjectInstrumentationConfig) {  // NOLINT
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

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

  auto file_reader = std::make_unique<FileReaderMock>();
  auto symbols_writer = std::make_unique<SymbolsWriterMock>();
  auto system_clock = std::make_unique<SystemClockMock>();
  InjectInstrumentation inject_instrumentation{{
      .inject_instrumentation = false,
      .file_reader = std::move(file_reader),
      .symbols_writer = std::move(symbols_writer),
      .system_clock = std::move(system_clock),
      .function_allow_list_file_path = {},
      .function_blocklist_file_path = {},
      .symbols_file_path = symbols_file_path,
      .module_id = {},
      .min_instruction_count_to_instrument = 0,
      .initialize_runtime = false,
      .enable_runtime = false,
  }};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_instrumentation.run(*parsed_module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(parsed_module.get(),
                       expected_uninstrumented_module.get());
  }
}

TEST(InjectInstrumentation, ReturnValue) {  // NOLINT
  struct alignas(64) TestCaseConfig {
    std::unordered_set<std::string> function_blocklist;
    bool are_all_preserved;
  };
  const std::vector<TestCaseConfig> configs{
      {.function_blocklist = {}, .are_all_preserved = false},
      {.function_blocklist = {"_Z9Fibonaccii"}, .are_all_preserved = true}};
  const std::filesystem::path blocklist_file_path{"/path/to/blocklist.txt"};
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

  for (const auto& config : configs) {
    llvm::SMDiagnostic module_diagnostic{};
    llvm::LLVMContext module_context{};
    auto parsed_module = llvm::parseIRFile(kNoMainIrFile.data(),
                                           module_diagnostic, module_context);
    ASSERT_NE(parsed_module, nullptr);

    std::stringstream blocklist{absl::StrJoin(config.function_blocklist, "\n")};
    auto file_reader = std::make_unique<FileReaderMock>();
    EXPECT_CALL(*file_reader, Open(blocklist_file_path));
    EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
    EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(blocklist));
    EXPECT_CALL(*file_reader, Close());
    auto symbols_writer = std::make_unique<SymbolsWriterMock>();
    EXPECT_CALL(*symbols_writer, Write(symbols_file_path, _))
        .WillOnce(Return(SymbolsWriterResult::Ok({})));
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectInstrumentation inject_instrumentation{{
        .inject_instrumentation = true,
        .file_reader = std::move(file_reader),
        .symbols_writer = std::move(symbols_writer),
        .system_clock = std::move(system_clock),
        .function_allow_list_file_path = {},
        .function_blocklist_file_path = blocklist_file_path,
        .symbols_file_path = symbols_file_path,
        .module_id = {},
        .min_instruction_count_to_instrument = 0,
        .initialize_runtime = false,
        .enable_runtime = false,
    }};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    const auto result =
        inject_instrumentation.run(*parsed_module, module_analysis_manager);
    ASSERT_EQ(result.areAllPreserved(), config.are_all_preserved);
  }
}

TEST(InjectInstrumentationDeathTest, FailsOnAllowListFileOpenError) {  // NOLINT
  const std::filesystem::path allow_list_file_path{"/path/to/allow_list.txt"};
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

  llvm::SMDiagnostic module_diagnostic{};
  llvm::LLVMContext module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         module_diagnostic, module_context);
  ASSERT_NE(parsed_module, nullptr);

  auto run = [&] {
    std::stringstream allow_list{};
    auto file_reader = std::make_unique<FileReaderMock>();
    EXPECT_CALL(*file_reader, Open(allow_list_file_path));
    EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(false));
    auto symbols_writer = std::make_unique<SymbolsWriterMock>();
    auto system_clock = std::make_unique<SystemClockMock>();
    InjectInstrumentation inject_instrumentation{{
        .inject_instrumentation = true,
        .file_reader = std::move(file_reader),
        .symbols_writer = std::move(symbols_writer),
        .system_clock = std::move(system_clock),
        .function_allow_list_file_path = allow_list_file_path,
        .function_blocklist_file_path = {},
        .symbols_file_path = symbols_file_path,
        .module_id = {},
        .min_instruction_count_to_instrument = 0,
        .initialize_runtime = false,
        .enable_runtime = false,
    }};

    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);
  };
  ASSERT_DEATH(  // NOLINT
      run(),
      "Failed to open the function allow list file '/path/to/allow_list.txt'.");
}

// NOLINTNEXTLINE
TEST(InjectInstrumentationDeathTest, FailsOnAllowListFileReadError) {
  const std::filesystem::path allow_list_file_path{"/path/to/allow_list.txt"};
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

  llvm::SMDiagnostic module_diagnostic{};
  llvm::LLVMContext module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         module_diagnostic, module_context);
  ASSERT_NE(parsed_module, nullptr);

  auto run = [&] {
    std::stringstream allow_list{};
    allow_list.setstate(std::ios::failbit);
    auto file_reader = std::make_unique<FileReaderMock>();
    EXPECT_CALL(*file_reader, Open(allow_list_file_path));
    EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
    EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(allow_list));
    EXPECT_CALL(*file_reader, Close());
    auto symbols_writer = std::make_unique<SymbolsWriterMock>();
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectInstrumentation inject_instrumentation{{
        .inject_instrumentation = true,
        .file_reader = std::move(file_reader),
        .symbols_writer = std::move(symbols_writer),
        .system_clock = std::move(system_clock),
        .function_allow_list_file_path = allow_list_file_path,
        .function_blocklist_file_path = {},
        .symbols_file_path = symbols_file_path,
        .module_id = {},
        .min_instruction_count_to_instrument = 0,
        .initialize_runtime = false,
        .enable_runtime = false,
    }};

    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);
  };
  ASSERT_DEATH(  // NOLINT
      run(),
      "Failed to read the function allow list file '/path/to/allow_list.txt'.");
}

// NOLINTNEXTLINE
TEST(InjectInstrumentationDeathTest, FailsOnBlocklistFileOpenError) {
  const std::filesystem::path blocklist_file_path{"/path/to/blocklist.txt"};
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

  llvm::SMDiagnostic module_diagnostic{};
  llvm::LLVMContext module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         module_diagnostic, module_context);
  ASSERT_NE(parsed_module, nullptr);

  auto run = [&] {
    std::stringstream blocklist{};
    auto file_reader = std::make_unique<FileReaderMock>();
    EXPECT_CALL(*file_reader, Open(blocklist_file_path));
    EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(false));
    auto symbols_writer = std::make_unique<SymbolsWriterMock>();
    auto system_clock = std::make_unique<SystemClockMock>();
    InjectInstrumentation inject_instrumentation{{
        .inject_instrumentation = true,
        .file_reader = std::move(file_reader),
        .symbols_writer = std::move(symbols_writer),
        .system_clock = std::move(system_clock),
        .function_allow_list_file_path = {},
        .function_blocklist_file_path = blocklist_file_path,
        .symbols_file_path = symbols_file_path,
        .module_id = {},
        .min_instruction_count_to_instrument = 0,
        .initialize_runtime = false,
        .enable_runtime = false,
    }};

    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);
  };
  ASSERT_DEATH(  // NOLINT
      run(),
      "Failed to open the function blocklist file '/path/to/blocklist.txt'.");
}

// NOLINTNEXTLINE
TEST(InjectInstrumentationDeathTest, FailsOnBlocklistFileReadError) {
  const std::filesystem::path blocklist_file_path{"/path/to/blocklist.txt"};
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

  llvm::SMDiagnostic module_diagnostic{};
  llvm::LLVMContext module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         module_diagnostic, module_context);
  ASSERT_NE(parsed_module, nullptr);

  auto run = [&] {
    std::stringstream blocklist{};
    blocklist.setstate(std::ios::failbit);
    auto file_reader = std::make_unique<FileReaderMock>();
    EXPECT_CALL(*file_reader, Open(blocklist_file_path));
    EXPECT_CALL(*file_reader, IsOpen()).WillOnce(Return(true));
    EXPECT_CALL(*file_reader, Istream()).WillOnce(ReturnRef(blocklist));
    EXPECT_CALL(*file_reader, Close());
    auto symbols_writer = std::make_unique<SymbolsWriterMock>();
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectInstrumentation inject_instrumentation{{
        .inject_instrumentation = true,
        .file_reader = std::move(file_reader),
        .symbols_writer = std::move(symbols_writer),
        .system_clock = std::move(system_clock),
        .function_allow_list_file_path = {},
        .function_blocklist_file_path = blocklist_file_path,
        .symbols_file_path = symbols_file_path,
        .module_id = {},
        .min_instruction_count_to_instrument = 0,
        .initialize_runtime = false,
        .enable_runtime = false,
    }};

    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);
  };
  ASSERT_DEATH(  // NOLINT
      run(),
      "Failed to read the function blocklist file '/path/to/blocklist.txt'.");
}

// NOLINTNEXTLINE
TEST(InjectInstrumentationDeathTest, FailsOnSymbolsFileOpenError) {
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

  llvm::SMDiagnostic module_diagnostic{};
  llvm::LLVMContext module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         module_diagnostic, module_context);
  ASSERT_NE(parsed_module, nullptr);

  auto run = [&] {
    std::stringstream blocklist{};
    blocklist.setstate(std::ios::failbit);
    auto file_reader = std::make_unique<FileReaderMock>();
    auto symbols_writer = std::make_unique<SymbolsWriterMock>();
    EXPECT_CALL(*symbols_writer, Write(symbols_file_path, _))
        .WillOnce(Return(SymbolsWriter::Error::kFailedToOpenFile));
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectInstrumentation inject_instrumentation{{
        .inject_instrumentation = true,
        .file_reader = std::move(file_reader),
        .symbols_writer = std::move(symbols_writer),
        .system_clock = std::move(system_clock),
        .function_allow_list_file_path = {},
        .function_blocklist_file_path = {},
        .symbols_file_path = symbols_file_path,
        .module_id = {},
        .min_instruction_count_to_instrument = 0,
        .initialize_runtime = false,
        .enable_runtime = false,
    }};

    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);
  };
  ASSERT_DEATH(  // NOLINT
      run(), "Failed to open the symbols file '/path/to/file.spoor_symbols'.");
}

// NOLINTNEXTLINE
TEST(InjectInstrumentationDeathTest, FailsOnSymbolsFileWriteError) {
  const std::filesystem::path symbols_file_path{"/path/to/file.spoor_symbols"};

  llvm::SMDiagnostic module_diagnostic{};
  llvm::LLVMContext module_context{};
  auto parsed_module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                         module_diagnostic, module_context);
  ASSERT_NE(parsed_module, nullptr);

  auto run = [&] {
    std::stringstream blocklist{};
    blocklist.setstate(std::ios::failbit);
    auto file_reader = std::make_unique<FileReaderMock>();
    auto symbols_writer = std::make_unique<SymbolsWriterMock>();
    EXPECT_CALL(*symbols_writer, Write(symbols_file_path, _))
        .WillOnce(Return(SymbolsWriter::Error::kSerializationError));
    auto system_clock = std::make_unique<SystemClockMock>();
    EXPECT_CALL(*system_clock, Now())
        .WillRepeatedly(Return(MakeTimePoint<std::chrono::system_clock>(0)));
    InjectInstrumentation inject_instrumentation{{
        .inject_instrumentation = true,
        .file_reader = std::move(file_reader),
        .symbols_writer = std::move(symbols_writer),
        .system_clock = std::move(system_clock),
        .function_allow_list_file_path = {},
        .function_blocklist_file_path = {},
        .symbols_file_path = symbols_file_path,
        .module_id = {},
        .min_instruction_count_to_instrument = 0,
        .initialize_runtime = false,
        .enable_runtime = false,
    }};

    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_instrumentation.run(*parsed_module, module_analysis_manager);
  };
  ASSERT_DEATH(  // NOLINT
      run(),
      "Failed to write the instrumentation symbols to "
      "'/path/to/file.spoor_symbols'.");
}

}  // namespace
