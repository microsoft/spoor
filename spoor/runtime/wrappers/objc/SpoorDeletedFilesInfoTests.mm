// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorDeletedFilesInfo_private.h"

#import <XCTest/XCTest.h>

#include "spoor/runtime/runtime.h"

@interface SpoorDeletedFilesInfoTests : XCTestCase

@end

@implementation SpoorDeletedFilesInfoTests

- (void)testDeletedFiles {
  constexpr spoor::runtime::DeletedFilesInfo backingDeletedFilesInfo{.deleted_files = 10,
                                                                     .deleted_bytes = 35};
  const auto* deletedFilesInfo =
      [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo];
  XCTAssertEqual(deletedFilesInfo.deletedFiles, static_cast<int32_t>(10));
}

- (void)testDeletedBytes {
  constexpr spoor::runtime::DeletedFilesInfo backingDeletedFilesInfo{.deleted_files = 10,
                                                                     .deleted_bytes = 35};
  const auto* deletedFilesInfo =
      [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo];
  XCTAssertEqual(deletedFilesInfo.deletedBytes, static_cast<int64_t>(35));
}

- (void)testEqualSameObject {
  constexpr spoor::runtime::DeletedFilesInfo backingDeletedFilesInfo{.deleted_files = 10,
                                                                     .deleted_bytes = 35};
  const auto* deletedFilesInfo1 =
      [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo];
  const auto* deletedFilesInfo2 =
      [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo];
  XCTAssertEqualObjects(deletedFilesInfo1, deletedFilesInfo2);
}

- (void)testEqualDifferentObjects {
  constexpr spoor::runtime::DeletedFilesInfo backingDeletedFilesInfo1{.deleted_files = 10,
                                                                      .deleted_bytes = 35};
  constexpr spoor::runtime::DeletedFilesInfo backingDeletedFilesInfo2{.deleted_files = 10,
                                                                      .deleted_bytes = 35};
  const auto* deletedFilesInfo1 =
      [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo1];
  const auto* deletedFilesInfo2 =
      [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo2];
  XCTAssertEqualObjects(deletedFilesInfo1, deletedFilesInfo2);
}

- (void)testNotEqual {
  constexpr spoor::runtime::DeletedFilesInfo backingDeletedFilesInfo1{.deleted_files = 10,
                                                                      .deleted_bytes = 36};
  constexpr spoor::runtime::DeletedFilesInfo backingDeletedFilesInfo2{.deleted_files = 11,
                                                                      .deleted_bytes = 35};
  const auto* deletedFilesInfo1 =
      [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo1];
  const auto* deletedFilesInfo2 =
      [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo2];
  XCTAssertNotEqualObjects(deletedFilesInfo1, deletedFilesInfo2);
}

@end
