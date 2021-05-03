// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#import <XCTest/XCTest.h>
#import "SpoorRuntime.h"
#import "SpoorDeletedFilesInfo_private.h"
#import "SpoorConfig_private.h"

@interface SpoorRuntimeStubTests : XCTestCase

@end

@implementation SpoorRuntimeStubTests

-(void)testInitializeRuntime
{
  XCTAssertFalse([SpoorRuntime isRuntimeInitialized]);
  [SpoorRuntime initializeRuntime];
  XCTAssertFalse([SpoorRuntime isRuntimeInitialized]);
  [SpoorRuntime initializeRuntime];
  XCTAssertFalse([SpoorRuntime isRuntimeInitialized]);
}

-(void)testEnableRuntime
{
  XCTAssertFalse([SpoorRuntime isRuntimeEnabled]);
  [SpoorRuntime enableRuntime];
  XCTAssertFalse([SpoorRuntime isRuntimeEnabled]);
  [SpoorRuntime enableRuntime];
  XCTAssertFalse([SpoorRuntime isRuntimeEnabled]);
}

-(void)testLogEvent
{
  [SpoorRuntime logEvent:1 timestamp:2 payload1:3 payload2:4];
  [SpoorRuntime logEvent:1 payload1:2 payload2:3];
  [SpoorRuntime logFunctionEntryWithFunctionId:1];
  [SpoorRuntime logFunctionExitWithFunctionId:1];
}

-(void)testFlushTraceEvents
{
  [SpoorRuntime flushTraceEventsWithCallback:^{}];
  __block BOOL invokedCallback = NO;
  [SpoorRuntime flushTraceEventsWithCallback:^{
    invokedCallback = YES;
  }];
  XCTAssertTrue(invokedCallback);
}

-(void)testClearTraceEvents
{
  [SpoorRuntime clearTraceEvents];
}

-(void)testFlushedTraceFiles
{
  [SpoorRuntime flushedTraceFilesWithCallback:^(NSArray<NSString *> * _Nullable) {}];

  __block BOOL invokedCallback = NO;
  [SpoorRuntime flushedTraceFilesWithCallback:^(NSArray<NSString *> * _Nullable traceFilePaths) {
    invokedCallback = YES;
    XCTAssertNil(traceFilePaths);
  }];
  XCTAssertTrue(invokedCallback);
}

-(void)testDeleteFlushedTraceFilesOlderThan
{
  [SpoorRuntime deleteFlushedTraceFilesOlderThanDate:[NSDate distantPast]
                                            callback:^(const SpoorDeletedFilesInfo *) {}];

  constexpr _spoor_runtime_DeletedFilesInfo expected_deleted_files_info{
    .deleted_files = 0, .deleted_bytes = 0};
  SpoorDeletedFilesInfo *expectedDeletedFilesInfo = [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:expected_deleted_files_info];
  __block BOOL invokedCallback = NO;
  [SpoorRuntime deleteFlushedTraceFilesOlderThanDate:[NSDate distantPast]
                                            callback:^(const SpoorDeletedFilesInfo * deletedFilesInfo) {
    invokedCallback = YES;
    XCTAssertEqualObjects(deletedFilesInfo, expectedDeletedFilesInfo);
  }];
  XCTAssertTrue(invokedCallback);
}

-(void)testConfig
{
  constexpr _spoor_runtime_Config expected_config{
    .trace_file_path = nullptr,
    .session_id = 0,
    .thread_event_buffer_capacity = 0,
    .max_reserved_event_buffer_slice_capacity = 0,
    .max_dynamic_event_buffer_slice_capacity = 0,
    .reserved_event_pool_capacity = 0,
    .dynamic_event_pool_capacity = 0,
    .dynamic_event_slice_borrow_cas_attempts = 0,
    .event_buffer_retention_duration_nanoseconds = 0,
    .max_flush_buffer_to_file_attempts = 0,
    .flush_all_events = false};
  SpoorConfig *expectedConfig = [[SpoorConfig alloc] initWithConfig:expected_config];
  SpoorConfig *config = [SpoorRuntime config];
  XCTAssertEqualObjects(config, expectedConfig);
}

-(void)testIsStubImplementation
{
  XCTAssertTrue([SpoorRuntime isStubImplementation]);
}

@end
