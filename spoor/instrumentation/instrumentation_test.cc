// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/instrumentation.h"

#include <type_traits>
#include <utility>

#include "gtest/gtest.h"
#include "spoor/instrumentation/symbols/symbols.pb.h"

namespace {

using FunctionSymbolsMapType = std::remove_reference_t<
    decltype(std::declval<spoor::instrumentation::symbols::Symbols>()
                 .function_symbols_table())>;

static_assert(std::is_same_v<spoor::instrumentation::FunctionId,
                             FunctionSymbolsMapType::key_type>);

}  // namespace
