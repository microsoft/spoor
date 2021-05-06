// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#import <XCTest/XCTest.h>
#import "SpoorConfig_private.h"
#import "SpoorDeletedFilesInfo_private.h"
#import "SpoorRuntime.h"

@interface SpoorRuntimeStubTests : XCTestCase

@end

@implementation SpoorRuntimeStubTests

- (void)testInitializeRuntime {
  XCTAssertFalse([SpoorRuntime isRuntimeInitialized]);
  [SpoorRuntime initializeRuntime];
  XCTAssertFalse([SpoorRuntime isRuntimeInitialized]);
  [SpoorRuntime initializeRuntime];
  XCTAssertFalse([SpoorRuntime isRuntimeInitialized]);
}

- (void)testEnableRuntime {
  XCTAssertFalse([SpoorRuntime isRuntimeEnabled]);
  [SpoorRuntime enableRuntime];
  XCTAssertFalse([SpoorRuntime isRuntimeEnabled]);
  [SpoorRuntime enableRuntime];
  XCTAssertFalse([SpoorRuntime isRuntimeEnabled]);
}

- (void)testLogEvent {
  [SpoorRuntime logEvent:1 timestamp:2 payload1:3 payload2:4];
  [SpoorRuntime logEvent:1 payload1:2 payload2:3];
}

- (void)testFlushTraceEvents {
  [SpoorRuntime flushTraceEventsWithCallback:^{
  }];

  XCTestExpectation* expectation =
      [self expectationWithDescription:@"Flushed trace files callback invoked"];
  [SpoorRuntime flushTraceEventsWithCallback:^{
    [expectation fulfill];
  }];
  [self waitForExpectationsWithTimeout:5.0
                               handler:^(NSError* error) {
                                 if (error) {
                                   XCTFail(@"Expectation Failed with error: %@", error);
                                 }
                               }];
}

- (void)testClearTraceEvents {
  [SpoorRuntime clearTraceEvents];
}

- (void)testFlushedTraceFiles {
  [SpoorRuntime flushedTraceFilesWithCallback:^(const NSArray<NSString*>* _Nullable){
  }];

  XCTestExpectation* expectation =
      [self expectationWithDescription:@"Flushed trace files callback invoked"];
  [SpoorRuntime flushedTraceFilesWithCallback:^(const NSArray<NSString*>* traceFilePaths) {
    XCTAssertEqual(traceFilePaths.count, static_cast<unsigned long>(0));
    [expectation fulfill];
  }];
  [self waitForExpectationsWithTimeout:5.0
                               handler:^(NSError* error) {
                                 if (error) {
                                   XCTFail(@"Expectation Failed with error: %@", error);
                                 }
                               }];
}

- (void)testDeleteFlushedTraceFilesOlderThan {
  [SpoorRuntime deleteFlushedTraceFilesOlderThanDate:[NSDate distantPast]
                                            callback:^(const SpoorDeletedFilesInfo*){
                                            }];

  constexpr spoor::runtime::DeletedFilesInfo expected_deleted_files_info{.deleted_files = 0,
                                                                         .deleted_bytes = 0};
  SpoorDeletedFilesInfo* expectedDeletedFilesInfo =
      [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:expected_deleted_files_info];
  XCTestExpectation* expectation =
      [self expectationWithDescription:@"Delete flushed trace files callback invoked"];
  [SpoorRuntime
      deleteFlushedTraceFilesOlderThanDate:[NSDate distantPast]
                                  callback:^(const SpoorDeletedFilesInfo* deletedFilesInfo) {
                                    XCTAssertEqualObjects(deletedFilesInfo,
                                                          expectedDeletedFilesInfo);
                                    [expectation fulfill];
                                  }];
  [self waitForExpectationsWithTimeout:5.0
                               handler:^(NSError* error) {
                                 if (error) {
                                   XCTFail(@"Expectation Failed with error: %@", error);
                                 }
                               }];
}

- (void)testConfig {
  const spoor::runtime::Config expected_config{.trace_file_path = {},
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
  SpoorConfig* expectedConfig = [[SpoorConfig alloc] initWithConfig:expected_config];
  SpoorConfig* config = [SpoorRuntime config];
  XCTAssertEqualObjects(config, expectedConfig);
}

- (void)testIsStubImplementation {
  XCTAssertTrue([SpoorRuntime isStubImplementation]);
}

@end
