// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/instrumentation/config/output_language.h"

#include "gtest/gtest.h"

namespace {

using spoor::instrumentation::config::BinaryOutput;
using spoor::instrumentation::config::OutputLanguage;

static_assert(BinaryOutput(OutputLanguage::kBitcode));
static_assert(!BinaryOutput(OutputLanguage::kIr));

}  // namespace
