// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <vector>

#include "gsl/gsl"
#include "util/result.h"

namespace util::serialization {

template <class T>
class Serializer {
 public:
  enum class SerializeError {
    kMissingSerializeFunction,
    kSerializingBuffer,
  };

  enum class DeserializeError {
    kMissingDeserializeFunction,
    kMalformedData,
    kDeserializingBuffer,
  };

  using SizeType = typename std::vector<const char>::size_type;
  using SerializeResult =
      util::result::Result<gsl::span<const char>, SerializeError>;
  using DeserializeResult =
      util::result::Result<gsl::span<const T>, DeserializeError>;

  Serializer(std::function<std::array<char, sizeof(T)>(const T&)> serialize,
             std::function<T(gsl::span<const char>)> deserialize,
             SizeType initial_buffer_capacity);
  [[nodiscard]] auto Serialize(gsl::span<const T> unserialized)
      -> SerializeResult;
  [[nodiscard]] auto Deserialize(gsl::span<const char> serialized)
      -> DeserializeResult;

 private:
  std::vector<char> buffer_;
  std::function<std::array<char, sizeof(T)>(const T&)> serialize_;
  std::function<T(gsl::span<const char>)> deserialize_;
};

template <class T>
Serializer<T>::Serializer(
    std::function<std::array<char, sizeof(T)>(const T&)> serialize,
    std::function<T(gsl::span<const char>)> deserialize,
    SizeType initial_buffer_capacity)
    : serialize_{std::move(serialize)}, deserialize_{std::move(deserialize)} {
  buffer_.reserve(initial_buffer_capacity);
}

template <class T>
auto Serializer<T>::Serialize(const gsl::span<const T> unserialized)
    -> SerializeResult {
  if (unserialized.empty()) {
    return gsl::span<const char>{buffer_.data(), unserialized.size()};
  }
  if (serialize_ == nullptr) {
    return SerializeError::kMissingSerializeFunction;
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  if (unserialized.data() == reinterpret_cast<T*>(buffer_.data())) {
    return SerializeError::kSerializingBuffer;
  }
  buffer_.reserve(unserialized.size() * sizeof(T));
  buffer_.resize(0);
  for (const auto& item : unserialized) {
    const auto serialized = serialize_(item);
    std::copy(std::cbegin(serialized), std::cend(serialized),
              std::back_inserter(buffer_));
  }
  return gsl::span<const char>{buffer_.data(), buffer_.size()};
}

template <class T>
auto Serializer<T>::Deserialize(const gsl::span<const char> serialized)
    -> DeserializeResult {
  if (serialized.empty()) {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return gsl::span<const T>{reinterpret_cast<T*>(buffer_.data()),
                              serialized.size()};
  }
  if (deserialize_ == nullptr) {
    return DeserializeError::kMissingDeserializeFunction;
  }
  if (serialized.data() == buffer_.data()) {
    return DeserializeError::kDeserializingBuffer;
  }
  if (serialized.size() % sizeof(T) != 0) {
    return DeserializeError::kMalformedData;
  }
  buffer_.reserve(serialized.size());
  buffer_.resize(0);
  for (const auto* ptr = serialized.data();
       // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
       ptr < serialized.data() + serialized.size(); ptr += sizeof(T)) {
    const auto deserialized =
        deserialize_(gsl::span<const char>(ptr, sizeof(T)));
    const gsl::span<const char> span{
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        reinterpret_cast<const char*>(&deserialized), sizeof(T)};
    std::copy(std::cbegin(span), std::cend(span), std::back_inserter(buffer_));
  }
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  return gsl::span<const T>{reinterpret_cast<T*>(buffer_.data()),
                            buffer_.size() / sizeof(T)};
}

}  // namespace util::serialization
