// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorTypes.h"

#include <type_traits>

#include "spoor/runtime/runtime.h"

static_assert(std::is_same_v<SpoorDurationNanoseconds, spoor::runtime::DurationNanoseconds>);
static_assert(std::is_same_v<SpoorEventType, spoor::runtime::EventType>);
static_assert(std::is_same_v<SpoorSessionId, spoor::runtime::SessionId>);
static_assert(std::is_same_v<SpoorSizeType, spoor::runtime::SizeType>);
static_assert(std::is_same_v<SpoorTimestampNanoseconds, spoor::runtime::TimestampNanoseconds>);