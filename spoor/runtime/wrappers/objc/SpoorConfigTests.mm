// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#import <XCTest/XCTest.h>
#import "SpoorConfig_private.h"

@interface SpoorConfigTests : XCTestCase

@end

@implementation SpoorConfigTests

- (void)testTraceFilePath {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqualObjects(config.traceFilePath, @"/path/to/file");
}

- (void)testTraceFilePath_Nil {
  constexpr _spoor_runtime_Config backingConfig{.trace_file_path = nil,
                                                .session_id = 7,
                                                .thread_event_buffer_capacity = 10,
                                                .max_reserved_event_buffer_slice_capacity = 12,
                                                .max_dynamic_event_buffer_slice_capacity = 15,
                                                .reserved_event_pool_capacity = 20,
                                                .dynamic_event_pool_capacity = 23,
                                                .dynamic_event_slice_borrow_cas_attempts = 25,
                                                .event_buffer_retention_duration_nanoseconds = 30,
                                                .max_flush_buffer_to_file_attempts = 54,
                                                .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertNil(config.traceFilePath);
}

- (void)testSessionId {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.sessionId, (SpoorSessionId)7);
}

- (void)testThreadEventBufferCapacity {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.threadEventBufferCapacity, static_cast<SpoorSizeType>(10));
}

- (void)testMaxReservedEventBufferSliceCapacity {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.maxReservedEventBufferSliceCapacity, static_cast<SpoorSizeType>(12));
}

- (void)testMaxDynamicEventBufferSliceCapacity {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.maxDynamicEventBufferSliceCapacity, static_cast<SpoorSizeType>(15));
}

- (void)testReservedEventPoolCapacity {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.reservedEventPoolCapacity, static_cast<SpoorSizeType>(20));
}

- (void)testDynamicEventPoolCapacity {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.dynamicEventPoolCapacity, static_cast<SpoorSizeType>(23));
}

- (void)testDynamicEventSliceBorrowCASAttempts {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.dynamicEventSliceBorrowCASAttempts, static_cast<SpoorSizeType>(25));
}

- (void)testEventBufferRetentionDuration {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.eventBufferRetentionDuration, static_cast<SpoorDurationNanoseconds>(30));
}

- (void)testMaxFlushBufferToFileAttempts {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.maxFlushBufferToFileAttempts, static_cast<int32_t>(54));
}

- (void)testFlushAllEvents_True {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = true};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertTrue(config.flushAllEvents);
}

- (void)testFlushAllEvents_False {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertFalse(config.flushAllEvents);
}

- (void)testEqualSameObjects {
  constexpr _spoor_runtime_Config backingConfig{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config1 = [[SpoorConfig alloc] initWithConfig:backingConfig];
  SpoorConfig* config2 = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqualObjects(config1, config2);
}

- (void)testEqualDifferentObjects {
  constexpr _spoor_runtime_Config backingConfig1{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  constexpr _spoor_runtime_Config backingConfig2{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  SpoorConfig* config1 = [[SpoorConfig alloc] initWithConfig:backingConfig1];
  SpoorConfig* config2 = [[SpoorConfig alloc] initWithConfig:backingConfig2];
  XCTAssertEqualObjects(config1, config2);
}

- (void)testNotEqual {
  constexpr _spoor_runtime_Config backingConfig1{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 10,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 21,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = false};
  constexpr _spoor_runtime_Config backingConfig2{
      .trace_file_path = static_cast<const char*>("/path/to/file"),
      .session_id = 7,
      .thread_event_buffer_capacity = 11,
      .max_reserved_event_buffer_slice_capacity = 12,
      .max_dynamic_event_buffer_slice_capacity = 15,
      .reserved_event_pool_capacity = 20,
      .dynamic_event_pool_capacity = 23,
      .dynamic_event_slice_borrow_cas_attempts = 25,
      .event_buffer_retention_duration_nanoseconds = 30,
      .max_flush_buffer_to_file_attempts = 54,
      .flush_all_events = true};
  SpoorConfig* config1 = [[SpoorConfig alloc] initWithConfig:backingConfig1];
  SpoorConfig* config2 = [[SpoorConfig alloc] initWithConfig:backingConfig2];
  XCTAssertNotEqualObjects(config1, config2);
}
@end
