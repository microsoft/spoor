# Runtime

Configure Spoor's runtime with environment variables or a configuration file.

!!! info "Precedence"
    1. Environment variables.
    2. Config file.

## Source code

[spoor/runtime/config/][spoor-runtime-config]

## Runtime options

### Trace file path

Directory in which to save flushed trace files.

**Type:** `string`

**Default:** `.`

Source               | Key
-------------------- | ------------------------------
Environment variable | `SPOOR_RUNTIME_TRACE_FILE_PATH`
Config file          | `trace_file_path`

!!! tip "Environment variable expansion"
    Spoor automatically expands environment variables. A tilde (i.e., `~`)
    expands to the value of `$HOME`.

### Compression strategy

Strategy used to compress the trace file on flush.

**Type:** `string`

**Options:** `none` or `snappy`

**Default:** `snappy`

Source               | Key
-------------------- | ------------------------------------
Environment variable | `SPOOR_RUNTIME_COMPRESSION_STRATEGY`
Config file          | `compression_strategy`

!!! tip "Compression ratio"
    Snappy offers a ~3x compression ratio.

### Session ID

Session identifier used to differentiate between runs.

**Type:** `uint64`

**Default:** Random number (likely distinct for each launch)

Source               | Key
-------------------- | --------------------------
Environment variable | `SPOOR_RUNTIME_SESSION_ID`
Config file          | `session_id`

### Thread event buffer capacity

Maximum number of events (zero or more slices) a thread may hold in its buffer.

**Type:** `uint64`

**Default:** `10,000`

Source               | Key
-------------------- | -------------------------------------------
Environment variable | `SPOOR_RUNTIME_THEAD_EVENT_BUFFER_CAPACITY`
Config file          | `thread_event_buffer_capacity`

### Max reserved event buffer slice capacity

Maximum number of events that a thread event buffer may request at once from
the reserved pool. These events are stored in a contiguous slice of memory.

**Type:** `uint64`

**Default:** `1,000`

Source               | Key
-------------------- | --------------------------------------------------------
Environment variable | `SPOOR_RUNTIME_MAX_RESERVED_EVENT_BUFFER_SLICE_CAPACITY`
Config file          | `max_reserved_event_buffer_slice_capacity`

### Max dynamic event buffer slice capacity

Maximum number of events that a thread event buffer may request at once from the
dynamic pool. These events are stored in a contiguous slice of memory.

**Type:** `uint64`

**Default:** `1,000`

Source               | Key
-------------------- | -------------------------------------------------------
Environment variable | `SPOOR_RUNTIME_MAX_DYNAMIC_EVENT_BUFFER_SLICE_CAPACITY`
Config file          | `max_dynamic_event_buffer_slice_capacity`

### Reserved event pool capacity

Event object pool size. The reserved pool is (dynamically) allocated as a
continuous block of memory on initialization and released on deinitialization.

**Type:** `uint64`

**Default:** `0`

Source               | Key
-------------------- | --------------------------------------------
Environment variable | `SPOOR_RUNTIME_RESERVED_EVENT_POOL_CAPACITY`
Config file          | `reserved_event_pool_capacity`

### Dynamic event pool capacity

Maximum number of events that can be dynamically allocated.

**Type:** `uint64`

**Default:** `18,446,744,073,709,551,615` (i.e., `max(uint64)`)

Source               | Key
-------------------- | -------------------------------------------
Environment variable | `SPOOR_RUNTIME_DYNAMIC_EVENT_POOL_CAPACITY`
Config file          | `dynamic_event_pool_capacity`

### Dynamic event slice borrow CAS attempts

Number of [compare and swap][compare-and-swap] attempts to borrow a slice from
the dynamic buffer pool.

**Type:** `uint64`

**Default:** `1`

Source               | Key
-------------------- | --------------------------
Environment variable | `SPOOR_RUNTIME_DYNAMIC_EVENT_SLICE_BORROW_CAS_ATTEMPTS`
Config file          | `dynamic_event_slice_borrow_cas_attempts`

### Event buffer retention duration

Duration in nanoseconds to retain a thread's buffer after a thread ends (in
anticipation of a flush event).

This value is ignored when [flush all events][flush-all-events] is `true`.

**Type:** `int64`

**Default:** `0`

Source               | Key
-------------------- | -----------------------------------------------------------
Environment variable | `SPOOR_RUNTIME_EVENT_BUFFER_RETENTION_DURATION_NANOSECONDS`
Config file          | `event_buffer_retention_duration_nanoseconds`

### Max flush buffer to file attempts

Maximum number of attempts to flush a buffer before discarding it.

**Type:** `int32`

**Default:** `2`

Source               | Key
-------------------- | -------------------------------------------------
Environment variable | `SPOOR_RUNTIME_MAX_FLUSH_BUFFER_TO_FILE_ATTEMPTS`
Config file          | `max_flush_buffer_to_file_attempts`

### Flush all events

When `true`, flushes a buffer as soon as it fills. When `false`, the buffer acts
as a circular buffer and old events are overridden.

**Type:** `bool`

**Default:** `true`

Source               | Key
-------------------- | --------------------------
Environment variable | `SPOOR_RUNTIME_FLUSH_ALL_EVENTS`
Config file          | `flush_all_events`

## Configuration file

To set a configuration file, implement the symbol
`spoor::runtime::ConfigFilePath()`.

Return `std::nullopt` or link against `libspoor_runtime_default_config.a` to use
the default config.

```c++
// spoor_config.cc

namespace spoor::runtime {

auto ConfigFilePath() -> std::optional<std::string> {
  return "/path/to/spoor_runtime_config.toml";
}

}  // namespace spoor::runtime
```

**Example configuration file**

```toml
# spoor_runtime_config.toml

trace_file_path = "/path/to/trace/"
compression_strategy = "snappy"
session_id = 42
thread_event_buffer_capacity = 1_000_000
max_reserved_event_buffer_slice_capacity = 1_000_000
max_dynamic_event_buffer_slice_capacity = 1_000_000
reserved_event_pool_capacity = 10_000_000
dynamic_event_pool_capacity = 9_223_372_036_854_775_807
dynamic_event_slice_borrow_cas_attempts = 1
event_buffer_retention_duration_nanoseconds = 10_000_000_000
max_flush_buffer_to_file_attempts = 2
flush_all_events = true
```

!!! warning "Config file max value"
    Many configuration types are **unsigned** 64-bit integers, however, TOML
    only supports up to **signed** 64-bit integers. Therefore, the maximum
    possilbe value in the config file is
    `max(int64) = 9,223,372,036,854,775,807`, not
    `max(uint64) = 18,446,744,073,709,551,615`.

    See [microsoft/spoor/issues/219][toml-signed-integer-issue].

## Calculating memory impact

```
event size = 24 bytes
min memory impact = event_size * reserved event pool capacity
max memory impact = min memory impact + event size * dynamic event pool capacity
```

[compare-and-swap]: https://en.wikipedia.org/wiki/Compare-and-swap
[flush-all-events]: #flush-all-events
[spoor-runtime-config]: https://github.com/microsoft/spoor/tree/master/spoor/runtime/config
[toml-signed-integer-issue]: https://github.com/microsoft/spoor/issues/219
