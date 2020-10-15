#pragma once

namespace spoor::runtime::flush_queue {

template <class T>
class FlushQueue {
 public:
  virtual ~FlushQueue() = default;

  virtual auto Run() -> void = 0;
  virtual auto Enqueue(T&& item) -> void = 0;
  virtual auto DrainAndStop() -> void = 0;
  virtual auto Flush() -> void = 0;
  virtual auto Clear() -> void = 0;

 protected:
  FlushQueue() = default;
  FlushQueue(const FlushQueue&) = default;
  FlushQueue(FlushQueue&&) noexcept = default;
  auto operator=(const FlushQueue&) -> FlushQueue& = default;
  auto operator=(FlushQueue&&) noexcept -> FlushQueue& = default;
};

}  // namespace spoor::runtime::flush_queue
