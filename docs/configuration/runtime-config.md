# Runtime

Configure Spoor's runtime with environment variables.

## Source code

[spoor/runtime/config/][spoor-instrumentation-config]

## Calculating memory impact

```
event_size = 24 bytes
min_memory_impact = event_size * SPOOR_RUNTIME_RESERVED_EVENT_POOL_CAPACITY
max_memory_impact = min_memory_impact + event_size * SPOOR_RUNTIME_RESERVED_EVENT_POOL_CAPACITY
```

## Runtime options

### SPOOR_RUNTIME_TRACE_FILE_PATH

Directory in which to save flushed trace files. The path must exist, and end
with a trailing `/`.

**Default:** `.`

### SPOOR_RUNTIME_COMPRESSION_STRATEGY

Strategy used to compress the trace file on flush.

**Options:** `none` or `snappy`.

Snappy offers a ~3x compression ratio.

**Default:** `snappy`

### SPOOR_RUNTIME_SESSION_ID

Session identifier used to differentiate between runs.

**Default:** Random number (distinct for each launch)

### SPOOR_RUNTIME_THEAD_EVENT_BUFFER_CAPACITY

Maximum number of events (zero or more slices) a thread may hold in its buffer.

**Default:** `10000`

### SPOOR_RUNTIME_MAX_RESERVED_EVENT_BUFFER_SLICE_CAPACITY

Maximum number of events that a thread event buffer may request at once from
the reserved pool. These events are stored in a contiguous slice of memory.

**Default:** `1000`

### SPOOR_RUNTIME_MAX_DYNAMIC_EVENT_BUFFER_SLICE_CAPACITY

Maximum number of events that a thread event buffer may request at once from the
dynamic pool. These events are stored in a contiguous slice of memory.

**Default:** `1000`

### SPOOR_RUNTIME_RESERVED_EVENT_POOL_CAPACITY

Event object pool size. The reserved pool is (dynamically) allocated as a
continuous block of memory on initialization and released on deinitialization.

**Default:** `0`

### SPOOR_RUNTIME_DYNAMIC_EVENT_POOL_CAPACITY

Maximum number of events that can be dynamically allocated.

**Default:** `18446744073709551615` (i.e., `max(uint64)`)

### SPOOR_RUNTIME_DYNAMIC_EVENT_SLICE_BORROW_CAS_ATTEMPTS

Number of [compare and swap][compare_and_swap] attempts to borrow a slice from
the dynamic buffer pool.

**Default:** `1`

### SPOOR_RUNTIME_EVENT_BUFFER_RETENTION_DURATION_NANOSECONDS

Duration to retain a thread's buffer after a thread ends (in anticipation of a
flush event).

This value is ignored when `SPOOR_RUNTIME_FLUSH_ALL_EVENTS` is `true`.

**Default:** `0`

### SPOOR_RUNTIME_MAX_FLUSH_BUFFER_TO_FILE_ATTEMPTS

Maximum number of attempts to flush a buffer before discarding it.

**Default:** `2`

### SPOOR_RUNTIME_FLUSH_ALL_EVENTS

When `true`, flushes a buffer as soon as it fills. When `false`, the buffer acts
as a circular buffer and old events are overridden.

**Default:** `true`

[compare_and_swap]: https://en.wikipedia.org/wiki/Compare-and-swap
[config_h]: https://github.com/microsoft/spoor/blob/master/spoor/runtime/config/config.h
[spoor-runtime-config]: https://github.com/microsoft/spoor/tree/master/spoor/runtime/config
[trace_h]: https://github.com/microsoft/spoor/blob/master/spoor/runtime/trace/trace.h
