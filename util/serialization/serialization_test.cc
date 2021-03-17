// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <array>
#include <string_view>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "util/numeric.h"
#include "util/serialization/serializer.h"

namespace {

struct Packet {
  uint64 x;
  uint32 y;
  uint16 z;
};

using Serializer = util::serialization::Serializer<Packet>;

auto operator==(const Packet& lhs, const Packet& rhs) -> bool {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

auto Serialize(const Packet& packet) -> std::array<char, sizeof(Packet)> {
  std::array<char, sizeof(Packet)> serialized{};
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  auto* serialized_packet = reinterpret_cast<Packet*>(serialized.data());
  serialized_packet->x = packet.x;
  serialized_packet->y = packet.y;
  serialized_packet->z = packet.z;
  return serialized;
}

auto Deserialize(gsl::span<const char> serialized) -> Packet {
  const auto* serialized_packet =
      // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
      reinterpret_cast<const Packet*>(serialized.data());
  return Packet{.x = serialized_packet->x,
                .y = serialized_packet->y,
                .z = serialized_packet->z};
}

TEST(Serializer, SerializesAndDeserializes) {  // NOLINT
  constexpr std::array<Packet, 3> packets{{{.x = 1, .y = 2, .z = 3},
                                           {.x = 4, .y = 5, .z = 6},
                                           {.x = 7, .y = 8, .z = 9}}};
  Serializer serializer{Serialize, Deserialize, 0};
  const auto serialized_result =
      serializer.Serialize({packets.data(), packets.size()});
  ASSERT_TRUE(serialized_result.IsOk());
  const auto serialized_data_span = serialized_result.Ok();
  const std::vector<char> serialized_data(std::cbegin(serialized_data_span),
                                          std::cend(serialized_data_span));
  const auto deserialized_result = serializer.Deserialize(serialized_data);
  ASSERT_TRUE(deserialized_result.IsOk());
  const auto deserialized_packets = deserialized_result.Ok();
  ASSERT_EQ(packets.size(), deserialized_packets.size());
  ASSERT_TRUE(std::equal(std::cbegin(deserialized_packets),
                         std::cend(deserialized_packets),
                         std::cbegin(packets)));
}

TEST(Serializer, FailsWithoutSerializeAndDeserializeFunctions) {  // NOLINT
  Serializer serializer{nullptr, nullptr, 0};
  constexpr std::array<Packet, 1> unserialized{{{.x = 1, .y = 2, .z = 3}}};

  const auto serialize_result =
      serializer.Serialize({unserialized.data(), unserialized.size()});
  ASSERT_TRUE(serialize_result.IsErr());
  const auto serialize_error = serialize_result.Err();
  ASSERT_EQ(serialize_error,
            Serializer::SerializeError::kMissingSerializeFunction);
  constexpr std::string_view serialized{"x", sizeof(Packet)};
  const auto deserialize_result =
      serializer.Deserialize({serialized.data(), serialized.size()});
  ASSERT_TRUE(deserialize_result.IsErr());
  const auto deserialize_error = deserialize_result.Err();
  ASSERT_EQ(deserialize_error,
            Serializer::DeserializeError::kMissingDeserializeFunction);
}

TEST(Serializer, SerializesAndDeserializesEmpty) {  // NOLINT
  constexpr std::array<Packet, 0> packets{};
  constexpr std::array<const char, 0> serialized_packets{};
  Serializer serializer{Serialize, Deserialize, 0};
  const auto serialized_result =
      serializer.Serialize({packets.data(), packets.size()});
  ASSERT_TRUE(serialized_result.IsOk());
  const auto serialized = serialized_result.Ok();
  ASSERT_EQ(serialized.size(), 0);
  const auto deserialized_result = serializer.Deserialize(
      {serialized_packets.data(), serialized_packets.size()});
  ASSERT_TRUE(deserialized_result.IsOk());
  const auto deserialized = deserialized_result.Ok();
  ASSERT_EQ(deserialized.size(), 0);
}

TEST(Serializer, FailsToSerializeBuffer) {  // NOLINT
  constexpr std::string_view serialized{"x", sizeof(Packet)};
  Serializer serializer{Serialize, Deserialize, 0};
  const auto deserialize_result =
      serializer.Deserialize({serialized.data(), serialized.size()});
  ASSERT_TRUE(deserialize_result.IsOk());
  const auto deserialized = deserialize_result.Ok();
  const auto serialize_result =
      serializer.Serialize({deserialized.data(), deserialized.size()});
  ASSERT_TRUE(serialize_result.IsErr());
  const auto serialize_error = serialize_result.Err();
  ASSERT_EQ(serialize_error, Serializer::SerializeError::kSerializingBuffer);
}

TEST(Serializer, FailsToDeserializeBuffer) {  // NOLINT
  constexpr std::array<Packet, 1> packets{{{.x = 1, .y = 2, .z = 3}}};
  Serializer serializer{Serialize, Deserialize, 0};
  const auto serialize_result =
      serializer.Serialize({packets.data(), packets.size()});
  ASSERT_TRUE(serialize_result.IsOk());
  const auto serialized = serialize_result.Ok();
  const auto deserialize_result =
      serializer.Deserialize({serialized.data(), serialized.size()});
  ASSERT_TRUE(deserialize_result.IsErr());
  const auto deserialize_error = deserialize_result.Err();
  ASSERT_EQ(deserialize_error,
            Serializer::DeserializeError::kDeserializingBuffer);
}

TEST(Serializer, FailsToDeserializeMalformedData) {  // NOLINT
  constexpr std::array<std::string_view, 4> bad_data{{
      "x",
      "xxx",
      std::string_view{"x", sizeof(Packet) - 1},
      std::string_view{"x", sizeof(Packet) + 1},
  }};
  Serializer serializer{Serialize, Deserialize, 0};
  for (const auto& data : bad_data) {
    ASSERT_TRUE(data.size() % sizeof(Packet) != 0);
    const auto decompress_result =
        serializer.Deserialize({data.data(), data.size()});
    ASSERT_TRUE(decompress_result.IsErr());
    const auto error = decompress_result.Err();
    ASSERT_EQ(error, Serializer::DeserializeError::kMalformedData);
  }
}

}  // namespace
