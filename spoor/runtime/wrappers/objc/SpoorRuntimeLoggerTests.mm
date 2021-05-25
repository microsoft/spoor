// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import <XCTest/XCTest.h>

#include "spoor/runtime/runtime.h"

#import "SpoorConfig_private.h"
#import "SpoorDeletedFilesInfo_private.h"
#import "SpoorRuntimeLogger.h"

@interface SpoorRuntimeTests : XCTestCase

@end

@implementation SpoorRuntimeTests

constexpr NSTimeInterval timeoutInterval{5.0};

- (void)testSetUp {
  for (auto iteration{0}; iteration < 3; ++iteration) {
    XCTAssertFalse([SpoorRuntime isLoggingEnabled]);
    XCTAssertFalse([SpoorRuntime isSetUp]);
    [SpoorRuntime setUp];
    XCTAssertFalse([SpoorRuntime isLoggingEnabled]);
    XCTAssertTrue([SpoorRuntime isSetUp]);
    [SpoorRuntime tearDown];
    XCTAssertFalse([SpoorRuntime isLoggingEnabled]);
    XCTAssertFalse([SpoorRuntime isSetUp]);
    [SpoorRuntime tearDown];
    XCTAssertFalse([SpoorRuntime isLoggingEnabled]);
    XCTAssertFalse([SpoorRuntime isSetUp]);
  }
}

- (void)testEnableLogging {
  XCTAssertFalse([SpoorRuntime isLoggingEnabled]);
  [SpoorRuntime setUp];
  XCTAssertFalse([SpoorRuntime isLoggingEnabled]);
  for (auto iteration{0}; iteration < 3; ++iteration) {
    [SpoorRuntime enableLogging];
    XCTAssertTrue([SpoorRuntime isLoggingEnabled]);
    [SpoorRuntime enableLogging];
    XCTAssertTrue([SpoorRuntime isLoggingEnabled]);
    [SpoorRuntime disableLogging];
    XCTAssertFalse([SpoorRuntime isLoggingEnabled]);
    [SpoorRuntime disableLogging];
    XCTAssertFalse([SpoorRuntime isLoggingEnabled]);
  }
  [SpoorRuntime enableLogging];
  XCTAssertTrue([SpoorRuntime isLoggingEnabled]);
  [SpoorRuntime tearDown];
  XCTAssertFalse([SpoorRuntime isLoggingEnabled]);
}

- (void)testFlushTraceEvents {
  [SpoorRuntime flushTraceEventsWithCallback:^{
  }];
}

- (void)testClearTraceEvents {
  [SpoorRuntime tearDown];
  [SpoorRuntime clearTraceEvents];
  [SpoorRuntime setUp];
  [SpoorRuntime clearTraceEvents];
  [SpoorRuntime tearDown];
}

- (void)testFlushedTraceFiles {
  XCTestExpectation* expectation =
      [self expectationWithDescription:@"Flushed trace files callback invoked"];
  [SpoorRuntime flushedTraceFilesWithCallback:^(const NSArray<NSString*>* traceFilePaths) {
    XCTAssertEqual(traceFilePaths.count, static_cast<unsigned long>(0));
    [expectation fulfill];
  }];
  [self waitForExpectationsWithTimeout:timeoutInterval
                               handler:^(NSError* error) {
                                 if (error != nil) {
                                   XCTFail(@"Expectation Failed with error: %@", error);
                                 }
                               }];
}

- (void)testDeleteFlushedTraceFilesOlderThan {
  constexpr spoor::runtime::DeletedFilesInfo expected_deleted_files_info{.deleted_files = 0,
                                                                         .deleted_bytes = 0};
  const auto* expectedDeletedFilesInfo =
      [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:expected_deleted_files_info];
  XCTestExpectation* expectation =
      [self expectationWithDescription:@"Delete flushed trace files callback invoked"];
  [SpoorRuntime
      deleteFlushedTraceFilesOlderThanDate:[NSDate distantFuture]
                                  callback:^(const SpoorDeletedFilesInfo* deletedFilesInfo) {
                                    XCTAssertEqualObjects(deletedFilesInfo,
                                                          expectedDeletedFilesInfo);
                                    [expectation fulfill];
                                  }];
  [self waitForExpectationsWithTimeout:timeoutInterval
                               handler:^(NSError* error) {
                                 if (error != nil) {
                                   XCTFail(@"Expectation Failed with error: %@", error);
                                 }
                               }];
}

- (void)testGetConfig {
  const spoor::runtime::Config expected_config{};
  const auto* expectedConfig = [[SpoorConfig alloc] initWithConfig:expected_config];
  const auto* config = [SpoorRuntime config];
  XCTAssertNotEqualObjects(config, expectedConfig);
}

- (void)testIsStubImplementation {
  XCTAssertFalse([SpoorRuntime isStubImplementation]);
}

@end
