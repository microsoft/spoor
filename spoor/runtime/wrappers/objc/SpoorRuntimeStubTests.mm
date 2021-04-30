// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-statement-expression"

#include "spoor/runtime/runtime.h"

#import <XCTest/XCTest.h>
#import "SpoorRuntime.h"
#import "SpoorTraceFiles_private.h"
#import "SpoorDeletedFilesInfo_private.h"
#import "SpoorConfig_private.h"

@interface SpoorRuntimeStubTests : XCTestCase

@end

@implementation SpoorRuntimeStubTests

-(void)testInitializeRuntime
{
  XCTAssertFalse([SpoorRuntime runtimeInitialized]);
  [SpoorRuntime initializeRuntime];
  XCTAssertFalse([SpoorRuntime runtimeInitialized]);
  [SpoorRuntime initializeRuntime];
  XCTAssertFalse([SpoorRuntime runtimeInitialized]);
}

-(void)testEnableRuntime
{
  XCTAssertFalse([SpoorRuntime runtimeEnabled]);
  [SpoorRuntime enableRuntime];
  XCTAssertFalse([SpoorRuntime runtimeEnabled]);
  [SpoorRuntime enableRuntime];
  XCTAssertFalse([SpoorRuntime runtimeEnabled]);
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
  __block BOOL invokedCallback;
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
  [SpoorRuntime flushedTraceFilesWithCallback:^(SpoorTraceFiles * _Nonnull) {}];

  _spoor_runtime_TraceFiles expected_trace_files{
    .file_paths_size = 0, .file_path_sizes = nullptr, .file_paths = nullptr};
  SpoorTraceFiles *expectedTraceFiles = [[SpoorTraceFiles alloc] initWithTraceFiles:expected_trace_files];
  __block BOOL invokedCallback;
  [SpoorRuntime flushedTraceFilesWithCallback:^(SpoorTraceFiles * _Nonnull traceFiles) {
    invokedCallback = YES;
    XCTAssertEqualObjects(traceFiles, expectedTraceFiles);
  }];
  XCTAssertTrue(invokedCallback);
}

-(void)testDeleteFlushedTraceFilesOlderThan
{
  [SpoorRuntime deleteFlushedTraceFilesOlderThanTimestamp:std::numeric_limits<_spoor_runtime_SystemTimestampSeconds>::min()
                                                 callback:^(SpoorDeletedFilesInfo * _Nonnull) {}];

  _spoor_runtime_DeletedFilesInfo expected_deleted_files_info{
    .deleted_files = 0, .deleted_bytes = 0};
  SpoorDeletedFilesInfo *expectedDeletedFilesInfo = [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:expected_deleted_files_info];
  __block BOOL invokedCallback;
  [SpoorRuntime deleteFlushedTraceFilesOlderThanTimestamp:std::numeric_limits<_spoor_runtime_SystemTimestampSeconds>::min()
                                                 callback:^(SpoorDeletedFilesInfo * _Nonnull deletedFilesInfo) {
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

-(void)testStubImplementation
{
  XCTAssertTrue([SpoorRuntime stubImplementation]);
}

@end

#pragma GCC diagnostic pop
