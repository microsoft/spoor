// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <type_traits>
#include "spoor/runtime/runtime.h"

#import "SpoorTypes.h"

static_assert(std::is_same_v<SpoorDurationNanoseconds, _spoor_runtime_DurationNanoseconds>);
static_assert(std::is_same_v<SpoorEventType, _spoor_runtime_EventType>);
static_assert(std::is_same_v<SpoorFunctionId, _spoor_runtime_FunctionId>);
static_assert(std::is_same_v<SpoorSessionId, _spoor_runtime_SessionId>);
static_assert(std::is_same_v<SpoorSizeType, _spoor_runtime_SizeType>);
static_assert(std::is_same_v<SpoorTimestampNanoseconds, _spoor_runtime_TimestampNanoseconds>);