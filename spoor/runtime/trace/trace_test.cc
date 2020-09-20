#include "spoor/runtime/trace/trace.h"

#include "gtest/gtest.h"

namespace {

using Type = spoor::runtime::trace::Event::Type;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::FunctionId;
using spoor::runtime::trace::TimestampNanoseconds;

TEST(Event, StoresProperties) {  // NOLINT
  const FunctionId function_id{0xffffffffffffffff};
  const TimestampNanoseconds timestamp{0x7fffffffffffffff};
  for (const auto type : {Type::kFunctionEntry, Type::kFunctionExit}) {
    const Event event{type, function_id, timestamp};
    ASSERT_EQ(event.GetType(), type);
    ASSERT_EQ(event.GetFunctionId(), function_id);
    ASSERT_EQ(event.GetTimestamp(), timestamp);
  }
}

TEST(Event, DropsMostSignificantTimestampBit) {
  const FunctionId function_id{0xffffffffffffffff};
  const TimestampNanoseconds timestamp{0xffffffffffffffff};
  const TimestampNanoseconds expected_timestamp{0x7fffffffffffffff};
  for (const auto type : {Type::kFunctionEntry, Type::kFunctionExit}) {
    const Event event{type, function_id, timestamp};
    ASSERT_EQ(event.GetType(), type);
    ASSERT_EQ(event.GetFunctionId(), function_id);
    ASSERT_EQ(event.GetTimestamp(), expected_timestamp);
  }
}

}  // namespace
