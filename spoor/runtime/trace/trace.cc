#include "spoor/runtime/trace/trace.h"

namespace spoor::runtime::trace {

Event::Event(const Type type, const FunctionId function_id,
             const TimestampNanoseconds timestamp)
    : function_id_{function_id},
      type_and_timestamp_{MakeTypeAndTimestamp(type, timestamp)} {}

auto Event::GetType() const -> Type {
  return static_cast<Type>(type_and_timestamp_ >> uint64{63});
}

auto Event::GetFunctionId() const -> FunctionId { return function_id_; }

auto Event::GetTimestamp() const -> TimestampNanoseconds {
  return type_and_timestamp_ & ~(uint64{1} << uint64{63});
}

auto Event::MakeTypeAndTimestamp(const Type type,
                                 const TimestampNanoseconds timestamp)
    -> uint64 {
  // const auto nanoseconds =
  // std::chrono::duration_cast<std::chrono::nanoseconds>(
  //                              timestamp.time_since_epoch())
  //                              .count();
  auto type_and_timestamp = static_cast<uint64>(type) << uint64{63};
  type_and_timestamp |=
      static_cast<uint64>(timestamp) & uint64{0x7fffffffffffffff};
  return type_and_timestamp;
}

}  // namespace spoor::runtime::trace
