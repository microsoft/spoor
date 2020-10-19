#include "spoor/instrumentation/inject_runtime/inject_runtime.h"

#include <functional>
#include <iterator>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "gsl/gsl"
#include "llvm/ADT/APInt.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/Passes/PassPlugin.h"
#include "spoor/instrumentation/instrumentation_map.pb.h"
#include "swift/Demangling/Demangle.h"
#include "swift/Demangling/Demangler.h"

namespace spoor::instrumentation::inject_runtime {

constexpr std::string_view kMainFunctionName{"main"};
constexpr std::string_view kInitializeRuntimeFunctionName{
    "__spoor_runtime_InitializeRuntime"};
constexpr std::string_view kDeinitializeRuntimeFunctionName{
    "__spoor_runtime_DeinitializeRuntime"};
constexpr std::string_view kEnableRuntimeFunctionName{
    "__spoor_runtime_EnableRuntime"};
constexpr std::string_view kLogFunctionEntryFunctionName{
    "__spoor_runtime_LogFunctionEntry"};
constexpr std::string_view kLogFunctionExitFunctionName{
    "__spoor_runtime_LogFunctionExit"};

InjectRuntime::InjectRuntime(const Options options) : options_{options} {}

auto InjectRuntime::run(llvm::Module& module, llvm::ModuleAnalysisManager &
                        /*unused*/) -> llvm::PreservedAnalyses {
  const auto [instrumented_function_map, modified] = InstrumentModule(&module);
  instrumented_function_map.SerializeToOstream(options_.ostream);

  if (modified) return llvm::PreservedAnalyses::none();
  return llvm::PreservedAnalyses::all();
}

auto InjectRuntime::InstrumentModule(gsl::not_null<llvm::Module*> module)
    -> std::pair<InstrumentedFunctionMap, bool> {
  InstrumentedFunctionMap instrumented_function_map{};
  instrumented_function_map.set_module_id(module->getModuleIdentifier());
  auto& function_map = *instrumented_function_map.mutable_function_map();

  auto& context = module->getContext();
  auto* void_return_type = llvm::Type::getVoidTy(context);
  constexpr bool variadic{true};
  llvm::ArrayRef<llvm::Type*> no_arguments{};

  auto* initialization_function_type =
      llvm::FunctionType::get(void_return_type, no_arguments, !variadic);
  const auto initialize_runtime = module->getOrInsertFunction(
      kInitializeRuntimeFunctionName.data(), initialization_function_type);
  const auto deinitialize_runtime = module->getOrInsertFunction(
      kDeinitializeRuntimeFunctionName.data(), initialization_function_type);

  auto* enable_function_type =
      llvm::FunctionType::get(void_return_type, no_arguments, !variadic);
  const auto enable_runtime = module->getOrInsertFunction(
      kEnableRuntimeFunctionName.data(), enable_function_type);

  llvm::ArrayRef<llvm::Type*> log_function_arguments{
      llvm::Type::getInt64Ty(context)};
  auto* log_function_type = llvm::FunctionType::get(
      void_return_type, log_function_arguments, !variadic);
  const auto log_function_entry = module->getOrInsertFunction(
      kLogFunctionEntryFunctionName.data(), log_function_type);
  const auto log_function_exit = module->getOrInsertFunction(
      kLogFunctionExitFunctionName.data(), log_function_type);

  uint64 function_id{0};
  bool modified{false};
  for (auto& function : module->functions()) {
    if (function.isDeclaration()) continue;
    const auto finally = gsl::finally([&] { ++function_id; });

    const auto function_name = function.getName();
    const auto demangled_name = [function_name] {
      if (swift::Demangle::isSwiftSymbol(function_name)) {
        return swift::Demangle::demangleSymbolAsString(
            function_name.data(), function_name.size(), {});
      }
      return llvm::demangle(function_name);
    }();

    const auto is_main = function_name.equals(kMainFunctionName.data());
    const auto instrument_function = [&] {
      if (is_main ||
          std::find(std::cbegin(options_.function_allow_list),
                    std::cend(options_.function_allow_list),
                    function_name) != std::cend(options_.function_allow_list)) {
        return true;
      }
      const auto instruction_count = function.getInstructionCount();
      if (instruction_count < options_.min_instruction_count_to_instrument) {
        return false;
      }
      return std::find(std::cbegin(options_.function_blocklist),
                       std::cend(options_.function_blocklist),
                       function_name) == std::cend(options_.function_blocklist);
    }();

    FunctionInfo function_info{};
    function_info.set_linkage_name(function_name);
    function_info.set_demangled_name(demangled_name);
    const auto* subprogram = function.getSubprogram();
    if (subprogram != nullptr) {
      function_info.set_file_name(subprogram->getFilename());
      function_info.set_directory(subprogram->getDirectory());
      function_info.set_line(subprogram->getLine());
    }
    function_info.set_instrumented(instrument_function);
    function_map[function_id] = function_info;

    modified |= instrument_function;
    if (!instrument_function) continue;

    llvm::IRBuilder builder{context};
    llvm::ArrayRef<llvm::Value*> args = {builder.getInt64(function_id)};
    builder.SetInsertPoint(&*function.getEntryBlock().getFirstInsertionPt());
    if (is_main) {
      if (options_.initialize_runtime) {
        builder.CreateCall(initialize_runtime);
        if (options_.enable_runtime) builder.CreateCall(enable_runtime);
      }
    }
    builder.CreateCall(log_function_entry, {args});
    for (auto& basic_block : function) {
      for (auto& instruction : basic_block) {
        const auto is_return_instruction =
            llvm::ReturnInst::classof(&instruction);
        if (is_return_instruction) {
          builder.SetInsertPoint(&instruction);
          builder.CreateCall(log_function_exit, {args});
          if (is_main) {
            builder.CreateCall(deinitialize_runtime);
          }
        }
      }
    }
  }
  return {instrumented_function_map, modified};
}

}  // namespace spoor::instrumentation::inject_runtime
