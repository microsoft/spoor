#include "spoor/instrumentation/inject_runtime/inject_runtime.h"

#include <filesystem>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "google/protobuf/util/message_differencer.h"
#include "gsl/gsl"
#include "gtest/gtest.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "spoor/instrumentation/instrumentation_map.pb.h"

namespace {

using google::protobuf::util::MessageDifferencer;
using spoor::instrumentation::InstrumentedFunctionInfo;
using spoor::instrumentation::InstrumentedFunctionMap;
using spoor::instrumentation::inject_runtime::InjectRuntime;

constexpr std::string_view kUninstrumentedIrFile{
    "spoor/instrumentation/inject_runtime/test_data/fib.ll"};
constexpr std::string_view kUninstrumentedIrWithDebugInfoFile{
    "spoor/instrumentation/inject_runtime/test_data/fib_debug.ll"};
constexpr std::string_view kInstrumentedIrFile{
    "spoor/instrumentation/inject_runtime/test_data/fib_instrumented.ll"};
constexpr std::string_view kInstrumentedInitializedIrFile{
    "spoor/instrumentation/inject_runtime/test_data/"
    "fib_instrumented_initialized.ll"};
constexpr std::string_view kInstrumentedInitializedEnabledIrFile{
    "spoor/instrumentation/inject_runtime/test_data/"
    "fib_instrumented_initialized_enabled.ll"};
constexpr std::string_view kOnlyMainFunctionInstrumentedIrFile{
    "spoor/instrumentation/inject_runtime/test_data/"
    "fib_only_main_instrumented.ll"};

auto AssertModulesEqual(gsl::not_null<llvm::Module*> module_a,
                        gsl::not_null<llvm::Module*> module_b) -> void {
  // Hack: Modules are equal if their IR string representations are equal,
  // except for the module identifier and source file name. A nice side effect
  // of this approach is that we get a diff in the error message.
  const std::string module_a_id{module_a->getModuleIdentifier()};
  module_a->setModuleIdentifier({});
  const std::string module_a_source_file{module_a->getSourceFileName()};
  module_a->setSourceFileName({});
  const std::string module_b_id{module_b->getModuleIdentifier()};
  module_b->setModuleIdentifier({});
  const std::string module_b_source_file{module_b->getSourceFileName()};
  module_b->setSourceFileName({});

  std::string module_a_ir{};
  llvm::raw_string_ostream module_a_ostream{module_a_ir};
  module_a_ostream << *module_a;

  std::string module_b_ir{};
  llvm::raw_string_ostream module_b_ostream{module_b_ir};
  module_b_ostream << *module_b;

  ASSERT_EQ(module_a_ir, module_b_ir);

  module_a->setModuleIdentifier(module_a_id);
  module_a->setSourceFileName(module_a_source_file);
  module_b->setModuleIdentifier(module_b_id);
  module_b->setSourceFileName(module_b_source_file);
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
    auto module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                    uninstrumented_module_diagnostic,
                                    uninstrumented_module_context);
    ASSERT_NE(module, nullptr);

    const std::filesystem::path file_path{};
    std::ostringstream ostream{};
    const std::unordered_set<std::string> function_blocklist{};
    const std::unordered_set<std::string> function_allow_list{};
    constexpr int32 min_instruction_count_to_instrument{0};
    const InjectRuntime::Options options{file_path,
                                         function_blocklist,
                                         function_allow_list,
                                         &ostream,
                                         min_instruction_count_to_instrument,
                                         config.initialize_runtime,
                                         config.enable_runtime};
    InjectRuntime inject_runtime{options};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_runtime.run(*module, module_analysis_manager);
    {
      SCOPED_TRACE("Modules are not equal.");
      AssertModulesEqual(module.get(), expected_instrumented_module.get());
    }
  }
}

TEST(InjectRuntime, OutputsInstrumentedFunctionMapWithDebugInfo) {  // NOLINT
  llvm::SMDiagnostic instrumented_module_diagnostic{};
  llvm::LLVMContext instrumented_module_context{};
  auto module = llvm::parseIRFile(kUninstrumentedIrWithDebugInfoFile.data(),
                                  instrumented_module_diagnostic,
                                  instrumented_module_context);
  ASSERT_NE(module, nullptr);

  const std::filesystem::path file_path{};
  std::stringstream stream{};
  const std::unordered_set<std::string> function_blocklist{};
  const std::unordered_set<std::string> function_allow_list{};
  constexpr int32 min_instruction_count_to_instrument{0};
  constexpr bool initialize_runtime{false};
  constexpr bool enable_runtime{true};
  const InjectRuntime::Options options{file_path,
                                       function_blocklist,
                                       function_allow_list,
                                       &stream,
                                       min_instruction_count_to_instrument,
                                       initialize_runtime,
                                       enable_runtime};
  InjectRuntime inject_runtime{options};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_runtime.run(*module, module_analysis_manager);

  InstrumentedFunctionMap expected_instrumented_function_map{};
  expected_instrumented_function_map.set_module_id(
      kUninstrumentedIrWithDebugInfoFile.data());
  auto* function_map =
      expected_instrumented_function_map.mutable_function_map();

  InstrumentedFunctionInfo fibonacci_function_info{};
  fibonacci_function_info.set_linkage_name("_Z9Fibonaccii");
  fibonacci_function_info.set_demangled_name("Fibonacci(int)");
  fibonacci_function_info.set_file_name("fibonacci.cc");
  fibonacci_function_info.set_directory("/path/to/file");
  fibonacci_function_info.set_line(1);
  fibonacci_function_info.set_instrumented(true);
  (*function_map)[0] = fibonacci_function_info;

  InstrumentedFunctionInfo main_function_info{};
  main_function_info.set_linkage_name("main");
  main_function_info.set_demangled_name("main");
  main_function_info.set_file_name("fibonacci.cc");
  main_function_info.set_directory("/path/to/file");
  main_function_info.set_line(6);
  main_function_info.set_instrumented(true);
  (*function_map)[1] = main_function_info;

  InstrumentedFunctionMap instrumented_function_map{};
  instrumented_function_map.ParseFromIstream(&stream);

  ASSERT_TRUE(MessageDifferencer::Equals(instrumented_function_map,
                                         expected_instrumented_function_map));
}

TEST(InjectRuntime, OutputsInstrumentedFunctionMapWithoutDebugInfo) {  // NOLINT
  llvm::SMDiagnostic instrumented_module_diagnostic{};
  llvm::LLVMContext instrumented_module_context{};
  auto module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                  instrumented_module_diagnostic,
                                  instrumented_module_context);
  ASSERT_NE(module, nullptr);

  const std::filesystem::path file_path{};
  std::stringstream stream{};
  const std::unordered_set<std::string> function_blocklist{};
  const std::unordered_set<std::string> function_allow_list{};
  constexpr int32 min_instruction_count_to_instrument{0};
  constexpr bool initialize_runtime{false};
  constexpr bool enable_runtime{true};
  const InjectRuntime::Options options{file_path,
                                       function_blocklist,
                                       function_allow_list,
                                       &stream,
                                       min_instruction_count_to_instrument,
                                       initialize_runtime,
                                       enable_runtime};
  InjectRuntime inject_runtime{options};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_runtime.run(*module, module_analysis_manager);

  InstrumentedFunctionMap expected_instrumented_function_map{};
  expected_instrumented_function_map.set_module_id(
      kUninstrumentedIrFile.data());
  auto* function_map =
      expected_instrumented_function_map.mutable_function_map();

  InstrumentedFunctionInfo fibonacci_function_info{};
  fibonacci_function_info.set_linkage_name("_Z9Fibonaccii");
  fibonacci_function_info.set_demangled_name("Fibonacci(int)");
  fibonacci_function_info.set_instrumented(true);
  (*function_map)[0] = fibonacci_function_info;

  InstrumentedFunctionInfo main_function_info{};
  main_function_info.set_linkage_name("main");
  main_function_info.set_demangled_name("main");
  main_function_info.set_instrumented(true);
  (*function_map)[1] = main_function_info;

  InstrumentedFunctionMap instrumented_function_map{};
  instrumented_function_map.ParseFromIstream(&stream);

  ASSERT_TRUE(MessageDifferencer::Equals(instrumented_function_map,
                                         expected_instrumented_function_map));
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
  auto module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                  uninstrumented_module_diagnostic,
                                  uninstrumented_module_context);
  ASSERT_NE(module, nullptr);

  const std::filesystem::path file_path{};
  std::ostringstream ostream{};
  const std::unordered_set<std::string> function_blocklist{"_Z9Fibonaccii"};
  const std::unordered_set<std::string> function_allow_list{};
  constexpr int32 min_instruction_count_to_instrument{0};
  constexpr bool initialize_runtime{false};
  constexpr bool enable_runtime{false};
  const InjectRuntime::Options options{file_path,
                                       function_blocklist,
                                       function_allow_list,
                                       &ostream,
                                       min_instruction_count_to_instrument,
                                       initialize_runtime,
                                       enable_runtime};
  InjectRuntime inject_runtime{options};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_runtime.run(*module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(module.get(), expected_instrumented_module.get());
  }
}

TEST(InjectRuntime, FunctionAllowList) {  // NOLINT
  llvm::SMDiagnostic expected_module_diagnostic{};
  llvm::LLVMContext expected_module_context{};
  auto expected_instrumented_module =
      llvm::parseIRFile(kInstrumentedIrFile.data(), expected_module_diagnostic,
                        expected_module_context);
  ASSERT_NE(expected_instrumented_module, nullptr);

  llvm::SMDiagnostic uninstrumented_module_diagnostic{};
  llvm::LLVMContext uninstrumented_module_context{};
  auto module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                  uninstrumented_module_diagnostic,
                                  uninstrumented_module_context);
  ASSERT_NE(module, nullptr);

  const std::filesystem::path file_path{};
  std::ostringstream ostream{};
  const std::unordered_set<std::string> function_blocklist{};
  const std::unordered_set<std::string> function_allow_list{"_Z9Fibonaccii"};
  constexpr auto min_instruction_count_to_instrument =
      std::numeric_limits<int32>::max();
  constexpr bool initialize_runtime{false};
  constexpr bool enable_runtime{false};
  const InjectRuntime::Options options{file_path,
                                       function_blocklist,
                                       function_allow_list,
                                       &ostream,
                                       min_instruction_count_to_instrument,
                                       initialize_runtime,
                                       enable_runtime};
  InjectRuntime inject_runtime{options};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_runtime.run(*module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(module.get(), expected_instrumented_module.get());
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
  auto module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                  uninstrumented_module_diagnostic,
                                  uninstrumented_module_context);
  ASSERT_NE(module, nullptr);

  const std::filesystem::path file_path{};
  std::ostringstream ostream{};
  const std::unordered_set<std::string> function_blocklist{"_Z9Fibonaccii"};
  const std::unordered_set<std::string> function_allow_list{"_Z9Fibonaccii"};
  constexpr int32 min_instruction_count_to_instrument{0};
  constexpr bool initialize_runtime{false};
  constexpr bool enable_runtime{false};
  const InjectRuntime::Options options{file_path,
                                       function_blocklist,
                                       function_allow_list,
                                       &ostream,
                                       min_instruction_count_to_instrument,
                                       initialize_runtime,
                                       enable_runtime};
  InjectRuntime inject_runtime{options};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_runtime.run(*module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(module.get(), expected_instrumented_module.get());
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
    auto module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                    uninstrumented_module_diagnostic,
                                    uninstrumented_module_context);
    ASSERT_NE(module, nullptr);

    const std::filesystem::path file_path{};
    std::ostringstream ostream{};
    const std::unordered_set<std::string> function_blocklist{};
    const std::unordered_set<std::string> function_allow_list{};
    constexpr bool initialize_runtime{false};
    constexpr bool enable_runtime{false};
    const InjectRuntime::Options options{
        file_path,     function_blocklist,           function_allow_list,
        &ostream,      config.instruction_threshold, initialize_runtime,
        enable_runtime};
    InjectRuntime inject_runtime{options};
    llvm::ModuleAnalysisManager module_analysis_manager{};
    inject_runtime.run(*module, module_analysis_manager);
    {
      SCOPED_TRACE("Modules are not equal.");
      AssertModulesEqual(module.get(), expected_instrumented_module.get());
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
  auto module = llvm::parseIRFile(kUninstrumentedIrFile.data(),
                                  uninstrumented_module_diagnostic,
                                  uninstrumented_module_context);
  ASSERT_NE(module, nullptr);

  const std::filesystem::path file_path{};
  std::ostringstream ostream{};
  const std::unordered_set<std::string> function_blocklist{"main",
                                                           "_Z9Fibonaccii"};
  const std::unordered_set<std::string> function_allow_list{};
  constexpr auto min_instruction_count_to_instrument =
      std::numeric_limits<int32>::max();
  constexpr bool initialize_runtime{false};
  constexpr bool enable_runtime{false};
  const InjectRuntime::Options options{file_path,
                                       function_blocklist,
                                       function_allow_list,
                                       &ostream,
                                       min_instruction_count_to_instrument,
                                       initialize_runtime,
                                       enable_runtime};
  InjectRuntime inject_runtime{options};
  llvm::ModuleAnalysisManager module_analysis_manager{};
  inject_runtime.run(*module, module_analysis_manager);
  {
    SCOPED_TRACE("Modules are not equal.");
    AssertModulesEqual(module.get(), expected_instrumented_module.get());
  }
}

}  // namespace
