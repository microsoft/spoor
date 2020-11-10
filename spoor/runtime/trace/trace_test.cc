#include "spoor/runtime/trace/trace.h"

#include "gtest/gtest.h"

namespace {

using spoor::runtime::trace::Deserialize;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::Footer;
using spoor::runtime::trace::FunctionId;
using spoor::runtime::trace::Header;
using spoor::runtime::trace::Serialize;
using spoor::runtime::trace::TimestampNanoseconds;
using Type = spoor::runtime::trace::Event::Type;

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

TEST(Event, DropsMostSignificantTimestampBit) {  // NOLINT
  const FunctionId function_id{0xffffffffffffffff};
  const auto timestamp = static_cast<TimestampNanoseconds>(0xffffffffffffffff);
  const TimestampNanoseconds expected_timestamp{0x7fffffffffffffff};
  for (const auto type : {Type::kFunctionEntry, Type::kFunctionExit}) {
    const Event event{type, function_id, timestamp};
    ASSERT_EQ(event.GetType(), type);
    ASSERT_EQ(event.GetFunctionId(), function_id);
    ASSERT_EQ(event.GetTimestamp(), expected_timestamp);
  }
}

TEST(Header, Serialization) {  // NOLINT
  Header expected_header{.version = 1,
                         .session_id = 2,
                         .process_id = 3,
                         .thread_id = 4,
                         .system_clock_timestamp = 5,
                         .steady_clock_timestamp = 6,
                         .event_count = 7};
  const auto serialized_header = Serialize(expected_header);
  const auto deserialized_header = Deserialize(serialized_header);
  ASSERT_EQ(deserialized_header, expected_header);
}

TEST(Event, Serialization) {  // NOLINT
  Event expected_event{Type::kFunctionEntry, 42, 7};
  const auto serialized_event = Serialize(expected_event);
  const auto deserialized_event = Deserialize(serialized_event);
  ASSERT_EQ(deserialized_event, expected_event);
}

TEST(Footer, Serialization) {  // NOLINT
  Footer expected_footer{};
  const auto serialized_footer = Serialize(expected_footer);
  const auto deserialized_footer = Deserialize(serialized_footer);
  ASSERT_EQ(deserialized_footer, expected_footer);
}

}  // namespace
