// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/inject_runtime/inject_runtime.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <functional>
#include <iterator>
#include <string_view>
#include <system_error>
#include <utility>

#include "city_hash/city.h"
#include "google/protobuf/timestamp.pb.h"
#include "google/protobuf/util/time_util.h"
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
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
#include "spoor/proto/spoor.pb.h"
#include "swift/Demangling/Demangle.h"
#include "swift/Demangling/Demangler.h"
#include "util/time/clock.h"

namespace spoor::instrumentation::inject_runtime {

using google::protobuf::util::TimeUtil;

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

InjectRuntime::InjectRuntime(Options&& options)
    : options_{std::move(options)} {}

auto InjectRuntime::run(llvm::Module& llvm_module, llvm::ModuleAnalysisManager&
                        /*unused*/) -> llvm::PreservedAnalyses {
  if (!options_.inject_instrumentation) return llvm::PreservedAnalyses::all();

  auto [instrumented_function_map, modified] = InstrumentModule(&llvm_module);

  const auto now = [&] {
    const auto now = options_.system_clock->Now().time_since_epoch();
    const auto nanoseconds =
        std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    return TimeUtil::NanosecondsToTimestamp(nanoseconds);
  }();
  *instrumented_function_map.mutable_created_at() = now;

  const auto file_name = [&] {
    std::string buffer{};
    const auto module_id =
        options_.module_id.value_or(llvm_module.getModuleIdentifier());
    const auto module_id_hash = CityHash64(module_id.data(), module_id.size());
    llvm::raw_string_ostream{buffer}
        << llvm::format_hex_no_prefix(module_id_hash,
                                      sizeof(module_id_hash) * 2)
        << '.' << kInstrumentedFunctionMapFileExtension;
    return buffer;
  }();
  const auto path = options_.instrumented_function_map_output_path / file_name;
  std::error_code error{};
  auto output_stream =
      options_.instrumented_function_map_output_stream(path.c_str(), &error);
  if (error) {
    llvm::WithColor::error();
    llvm::errs()
        << "Failed to open/create the instrumentation map output file '" << path
        << "'. " << error.message() << ".\n";
    exit(EXIT_FAILURE);
  }

  // LLVM passes do not support `std::ostream` so we're forced to bridge the
  // output with a `std::string` instead of directly using `SerializeToOstream`.
  std::string buffer{};
  instrumented_function_map.SerializeToString(&buffer);
  *output_stream << buffer;

  if (modified) return llvm::PreservedAnalyses::none();
  return llvm::PreservedAnalyses::all();
}

auto InjectRuntime::InstrumentModule(gsl::not_null<llvm::Module*> llvm_module)
    const -> std::pair<InstrumentedFunctionMap, bool> {
  const auto& module_id = llvm_module->getModuleIdentifier();
  InstrumentedFunctionMap instrumented_function_map{};
  instrumented_function_map.set_module_id(module_id);
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

  const auto make_function_id = [&module_id](const uint64 counter) {
    const auto module_id_hash = CityHash32(module_id.data(), module_id.size());
    constexpr uint64 partition{32};
    static_assert(sizeof(module_id_hash) * 8 == partition);
    return (static_cast<uint64>(module_id_hash) << partition) | counter;
  };

  uint64 counter{0};
  bool modified{false};
  for (auto& function : llvm_module->functions()) {
    if (function.isDeclaration()) continue;
    const auto function_id = make_function_id(counter);
    const auto finally = gsl::finally([&counter] { ++counter; });

    const auto function_name = function.getName().str();
    const auto demangled_name = [&function_name] {
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
