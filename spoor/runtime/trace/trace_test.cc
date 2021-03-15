// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/trace/trace.h"

#include "gtest/gtest.h"

namespace {

using spoor::runtime::trace::Deserialize;
using spoor::runtime::trace::Event;
using spoor::runtime::trace::EventType;
using spoor::runtime::trace::FunctionId;
using spoor::runtime::trace::Header;
using spoor::runtime::trace::Serialize;
using spoor::runtime::trace::TimestampNanoseconds;
using Type = spoor::runtime::trace::Event::Type;

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
  Event expected_event{.steady_clock_timestamp = 1,
                       .payload_1 = 2,
                       .type = static_cast<EventType>(Type::kFunctionEntry),
                       .payload_2 = 3};
  const auto serialized_event = Serialize(expected_event);
  const auto deserialized_event = Deserialize(serialized_event);
  ASSERT_EQ(deserialized_event, expected_event);
}

}  // namespace
