// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/inject_runtime/inject_runtime.h"

#include <algorithm>
#include <cctype>
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
    "_spoor_runtime_InitializeRuntime"};
constexpr std::string_view kDeinitializeRuntimeFunctionName{
    "_spoor_runtime_DeinitializeRuntime"};
constexpr std::string_view kEnableRuntimeFunctionName{
    "_spoor_runtime_EnableRuntime"};
constexpr std::string_view kLogFunctionEntryFunctionName{
    "_spoor_runtime_LogFunctionEntry"};
constexpr std::string_view kLogFunctionExitFunctionName{
    "_spoor_runtime_LogFunctionExit"};

InjectRuntime::InjectRuntime(Options options) : options_{std::move(options)} {}

auto InjectRuntime::run(llvm::Module& llvm_module, llvm::ModuleAnalysisManager&
                        /*unused*/) -> llvm::PreservedAnalyses {
  const auto [instrumented_function_map, modified] =
      InstrumentModule(&llvm_module);
  instrumented_function_map.SerializeToOstream(options_.ostream);

  if (modified) return llvm::PreservedAnalyses::none();
  return llvm::PreservedAnalyses::all();
}

auto InjectRuntime::InstrumentModule(gsl::not_null<llvm::Module*> llvm_module)
    const -> std::pair<InstrumentedFunctionMap, bool> {
  InstrumentedFunctionMap instrumented_function_map{};
  instrumented_function_map.set_module_id(llvm_module->getModuleIdentifier());
  auto& function_map = *instrumented_function_map.mutable_function_map();

  auto& context = llvm_module->getContext();
  auto* void_return_type = llvm::Type::getVoidTy(context);
  constexpr bool variadic{true};

  auto* initialization_function_type =
      llvm::FunctionType::get(void_return_type, {}, !variadic);
  const auto initialize_runtime = llvm_module->getOrInsertFunction(
      kInitializeRuntimeFunctionName.data(), initialization_function_type);
  const auto deinitialize_runtime = llvm_module->getOrInsertFunction(
      kDeinitializeRuntimeFunctionName.data(), initialization_function_type);

  auto* enable_function_type =
      llvm::FunctionType::get(void_return_type, {}, !variadic);
  const auto enable_runtime = llvm_module->getOrInsertFunction(
      kEnableRuntimeFunctionName.data(), enable_function_type);

  auto* log_function_type = llvm::FunctionType::get(
      void_return_type, {llvm::Type::getInt64Ty(context)}, !variadic);
  const auto log_function_entry = llvm_module->getOrInsertFunction(
      kLogFunctionEntryFunctionName.data(), log_function_type);
  const auto log_function_exit = llvm_module->getOrInsertFunction(
      kLogFunctionExitFunctionName.data(), log_function_type);

  uint64 function_id{0};
  bool modified{false};
  for (auto& function : llvm_module->functions()) {
    if (function.isDeclaration()) continue;
    const auto finally = gsl::finally([&] { ++function_id; });

    const auto function_name = function.getName().str();
    const auto demangled_name = [function_name] {
      if (swift::Demangle::isSwiftSymbol(function_name)) {
        return swift::Demangle::demangleSymbolAsString(
            function_name.c_str(), function_name.size(), {});
      }
      std::string sanitized_function_name{};
      std::copy_if(
          std::cbegin(function_name), std::cend(function_name),
          std::back_inserter(sanitized_function_name),
          [](const auto character) { return !std::iscntrl(character); });
      return llvm::demangle(sanitized_function_name);
    }();

    const auto is_main = function_name == kMainFunctionName;
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
      function_info.set_file_name(subprogram->getFilename().str());
      function_info.set_directory(subprogram->getDirectory().str());
      function_info.set_line(subprogram->getLine());
    }
    function_info.set_instrumented(instrument_function);
    function_map[function_id] = function_info;

    modified |= instrument_function;
    if (!instrument_function) continue;

    llvm::IRBuilder builder{context};
    builder.SetInsertPoint(&*function.getEntryBlock().getFirstInsertionPt());
    if (is_main) {
      if (options_.initialize_runtime) {
        builder.CreateCall(initialize_runtime);
        if (options_.enable_runtime) builder.CreateCall(enable_runtime);
      }
    }
    builder.CreateCall(log_function_entry, {builder.getInt64(function_id)});
    for (auto& basic_block : function) {
      for (auto& instruction : basic_block) {
        const auto is_return_instruction =
            llvm::ReturnInst::classof(&instruction);
        if (is_return_instruction) {
          builder.SetInsertPoint(&instruction);
          (void)log_function_exit;
          builder.CreateCall(log_function_exit,
                             {builder.getInt64(function_id)});
          if (is_main) builder.CreateCall(deinitialize_runtime);
        }
      }
    }
  }
  return {instrumented_function_map, modified};
}

}  // namespace spoor::instrumentation::inject_runtime
