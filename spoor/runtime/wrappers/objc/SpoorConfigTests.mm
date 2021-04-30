// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-statement-expression"

#include "spoor/runtime/runtime.h"

#import <XCTest/XCTest.h>
#import "SpoorConfig_private.h"

@interface SpoorConfigTests : XCTestCase

@end

@implementation SpoorConfigTests

-(void)testTraceFilePathSize
{
  constexpr _spoor_runtime_Config backingConfig{
    .trace_file_path_size = 4};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.traceFilePathSize, (SpoorSizeType)4);
}

-(void)testTraceFilePath
{
  constexpr _spoor_runtime_Config backingConfig{
    .trace_file_path = (char*)"/path/to/file"};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqualObjects(config.traceFilePath, @"/path/to/file");
}

-(void)testTraceFilePathNil
{
  constexpr _spoor_runtime_Config backingConfig{
    .trace_file_path = nil};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertNil(config.traceFilePath);
}

-(void)testSessionId
{
  constexpr _spoor_runtime_Config backingConfig{
    .session_id = 7};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.sessionId, (SpoorSessionId)7);
}

-(void)testThreadEventBufferCapacity
{
  constexpr _spoor_runtime_Config backingConfig{
    .thread_event_buffer_capacity = 10};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.threadEventBufferCapacity, (SpoorSizeType)10);
}

-(void)testMaxReservedEventBufferSliceCapacity
{
  constexpr _spoor_runtime_Config backingConfig{
    .max_reserved_event_buffer_slice_capacity = 12};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.maxReservedEventBufferSliceCapacity, (SpoorSizeType)12);
}

-(void)testReservedEventPoolCapacity
{
  constexpr _spoor_runtime_Config backingConfig{
    .reserved_event_pool_capacity = 15};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.reservedEventPoolCapacity, (SpoorSizeType)15);
}

-(void)testDynamicEventPoolCapacity
{
  constexpr _spoor_runtime_Config backingConfig{
    .dynamic_event_pool_capacity = 20};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.dynamicEventPoolCapacity, (SpoorSizeType)20);
}

-(void)testDynamicEventSliceBorrowCASAttempts
{
  constexpr _spoor_runtime_Config backingConfig{
    .dynamic_event_slice_borrow_cas_attempts = 23};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.dynamicEventSliceBorrowCASAttempts, (SpoorSizeType)23);
}

-(void)testEventBufferRetentionDurationNanoseconds
{
  constexpr _spoor_runtime_Config backingConfig{
    .event_buffer_retention_duration_nanoseconds = 27};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.eventBufferRetentionDurationNanoseconds, (SpoorDurationNanoseconds)27);
}

-(void)testMaxFlushBufferToFileAttempts
{
  constexpr _spoor_runtime_Config backingConfig{
    .max_flush_buffer_to_file_attempts = 29};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqual(config.maxFlushBufferToFileAttempts, (int32_t)29);
}

-(void)testFlushAllEvents_True
{
  constexpr _spoor_runtime_Config backingConfig{
    .flush_all_events = true};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertTrue(config.flushAllEvents);
}

-(void)testFlushAllEvents_False
{
  constexpr _spoor_runtime_Config backingConfig{
    .flush_all_events = false};
  SpoorConfig *config = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertFalse(config.flushAllEvents);
}

-(void)testEqualSameObjects
{
  constexpr _spoor_runtime_Config backingConfig{
    .max_reserved_event_buffer_slice_capacity = 12,
    .dynamic_event_slice_borrow_cas_attempts = 1,
    .max_flush_buffer_to_file_attempts = 30,
  };
  SpoorConfig *config1 = [[SpoorConfig alloc] initWithConfig:backingConfig];
  SpoorConfig *config2 = [[SpoorConfig alloc] initWithConfig:backingConfig];
  XCTAssertEqualObjects(config1, config2);
}

-(void)testEqualDifferentObjects
{
  constexpr _spoor_runtime_Config backingConfig1{
    .max_reserved_event_buffer_slice_capacity = 12,
    .dynamic_event_slice_borrow_cas_attempts = 1,
    .max_flush_buffer_to_file_attempts = 30,
  };
  constexpr _spoor_runtime_Config backingConfig2{
    .max_reserved_event_buffer_slice_capacity = 12,
    .dynamic_event_slice_borrow_cas_attempts = 1,
    .max_flush_buffer_to_file_attempts = 30,
  };
  SpoorConfig *config1 = [[SpoorConfig alloc] initWithConfig:backingConfig1];
  SpoorConfig *config2 = [[SpoorConfig alloc] initWithConfig:backingConfig2];
  XCTAssertEqualObjects(config1, config2);
}

-(void)testNotEqual
{
  constexpr _spoor_runtime_Config backingConfig1{
    .max_reserved_event_buffer_slice_capacity = 12,
    .dynamic_event_slice_borrow_cas_attempts = 1,
    .max_flush_buffer_to_file_attempts = 30,
  };
  constexpr _spoor_runtime_Config backingConfig2{
    .max_reserved_event_buffer_slice_capacity = 13,
    .dynamic_event_slice_borrow_cas_attempts = 2,
    .max_flush_buffer_to_file_attempts = 31,
  };
  SpoorConfig *config1 = [[SpoorConfig alloc] initWithConfig:backingConfig1];
  SpoorConfig *config2 = [[SpoorConfig alloc] initWithConfig:backingConfig2];
  XCTAssertNotEqualObjects(config1, config2);
}
@end

#pragma GCC diagnostic pop
