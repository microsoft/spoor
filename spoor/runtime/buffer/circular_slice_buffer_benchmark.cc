#include <limits>

#include "benchmark/benchmark.h"
#include "gsl/gsl"
#include "spoor/runtime/buffer/amalgamated_buffer_slice_pool.h"
#include "spoor/runtime/buffer/circular_slice_buffer.h"
#include "util/numeric.h"

namespace {

using Pool = spoor::runtime::buffer::AmalgamatedBufferSlicePool<int8>;
using ValueType = Pool::ValueType;
using SizeType = Pool::SizeType;
using CircularSliceBuffer =
    spoor::runtime::buffer::CircularSliceBuffer<ValueType>;

auto CustomArguments(benchmark::internal::Benchmark* benchmark) -> void {
  for (SizeType circular_buffer_slice_capacity{8};
       circular_buffer_slice_capacity <= (8 << 15);
       circular_buffer_slice_capacity *= 8) {
    for (SizeType max_slice_capacity{circular_buffer_slice_capacity / 4};
         max_slice_capacity <= circular_buffer_slice_capacity;
         max_slice_capacity += circular_buffer_slice_capacity / 4) {
      for (SizeType dynamic_pool_capacity{0};
           dynamic_pool_capacity <= circular_buffer_slice_capacity;
           dynamic_pool_capacity += circular_buffer_slice_capacity / 2) {
        const auto reserved_pool_capacity =
            circular_buffer_slice_capacity - dynamic_pool_capacity;
        benchmark->Args(
            {gsl::narrow_cast<int64>(circular_buffer_slice_capacity),
             gsl::narrow_cast<int64>(max_slice_capacity),
             gsl::narrow_cast<int64>(reserved_pool_capacity),
             gsl::narrow_cast<int64>(dynamic_pool_capacity)});
      }
    }
  }
}

auto BenchmarkCircularSliceBufferPush(benchmark::State& state) -> void {
  const SizeType capacity{4096};
  const auto dynamic_pool_capacity = static_cast<SizeType>(state.range(0));
  const auto max_slice_capacity = static_cast<SizeType>(state.range(1));
  const typename Pool::Options pool_options{
      .reserved_pool_options = {.max_slice_capacity = max_slice_capacity,
                                .capacity = capacity - dynamic_pool_capacity},
      .dynamic_pool_options = {.max_slice_capacity = max_slice_capacity,
                               .capacity = dynamic_pool_capacity,
                               .borrow_cas_attempts =
                                   std::numeric_limits<SizeType>::max()},
  };
  Pool pool{pool_options};
  const typename CircularSliceBuffer::Options circular_slice_buffer_options{
      .buffer_slice_pool = &pool,
      .capacity = capacity,
  };
  CircularSliceBuffer circular_slice_buffer{circular_slice_buffer_options};
  for (auto _ : state) {
    circular_slice_buffer.Push(42);
  }
}
BENCHMARK(BenchmarkCircularSliceBufferPush)
    ->ArgsProduct({{0, 2048, 4096}, {8, 512, 1024}});

auto BenchmarkCircularSliceBufferCreateFillClear(benchmark::State& state)
    -> void {
  const typename Pool::Options pool_options{
      .reserved_pool_options = {.max_slice_capacity =
                                    static_cast<SizeType>(state.range(1)),
                                .capacity =
                                    static_cast<SizeType>(state.range(2))},
      .dynamic_pool_options =
          {.max_slice_capacity = static_cast<SizeType>(state.range(1)),
           .capacity = static_cast<SizeType>(state.range(3)),
           .borrow_cas_attempts = std::numeric_limits<SizeType>::max()},
  };
  Pool pool{pool_options};
  const typename CircularSliceBuffer::Options circular_slice_buffer_options{
      .buffer_slice_pool = &pool,
      .capacity = gsl::narrow_cast<SizeType>(state.range(0)),
  };
  for (auto _ : state) {
    CircularSliceBuffer circular_slice_buffer{circular_slice_buffer_options};
    for (SizeType i{0}; i < circular_slice_buffer_options.capacity; ++i) {
      circular_slice_buffer.Push(42);
    }
    circular_slice_buffer.Clear();
  }
}
BENCHMARK(BenchmarkCircularSliceBufferCreateFillClear)->Apply(CustomArguments);

}  // namespace

BENCHMARK_MAIN();
