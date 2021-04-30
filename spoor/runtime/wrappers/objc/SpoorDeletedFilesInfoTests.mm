// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-statement-expression"

#include "spoor/runtime/runtime.h"

#import <XCTest/XCTest.h>
#import "SpoorDeletedFilesInfo_private.h"

@interface SpoorDeletedFilesInfoTests : XCTestCase

@end

@implementation SpoorDeletedFilesInfoTests

-(void)testDeletedFiles
{
  constexpr _spoor_runtime_DeletedFilesInfo backingDeletedFilesInfo = {
    .deleted_files = 10};
  SpoorDeletedFilesInfo *deletedFilesInfo = [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo];
  XCTAssertEqual(deletedFilesInfo.deletedFiles, (int32_t)10);
}

-(void)testDeletedBytes
{
  constexpr _spoor_runtime_DeletedFilesInfo backingDeletedFilesInfo = {
    .deleted_bytes = 35};
  SpoorDeletedFilesInfo *deletedFilesInfo = [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo];
  XCTAssertEqual(deletedFilesInfo.deletedBytes, (int64_t)35);
}

-(void)testEqualSameObject
{
  constexpr _spoor_runtime_DeletedFilesInfo backingDeletedFilesInfo = {
    .deleted_files = 15,
    .deleted_bytes = 20,
  };
  SpoorDeletedFilesInfo *deletedFilesInfo1 = [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo];
  SpoorDeletedFilesInfo *deletedFilesInfo2 = [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo];
  XCTAssertEqualObjects(deletedFilesInfo1, deletedFilesInfo2);
}

-(void)testEqualDifferentObjects
{
  constexpr _spoor_runtime_DeletedFilesInfo backingDeletedFilesInfo1 = {
    .deleted_files = 15,
    .deleted_bytes = 20,
  };
  constexpr _spoor_runtime_DeletedFilesInfo backingDeletedFilesInfo2 = {
    .deleted_files = 15,
    .deleted_bytes = 20,
  };
  SpoorDeletedFilesInfo *deletedFilesInfo1 = [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo1];
  SpoorDeletedFilesInfo *deletedFilesInfo2 = [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo2];
  XCTAssertEqualObjects(deletedFilesInfo1, deletedFilesInfo2);
}

-(void)testNotEqual
{
  constexpr _spoor_runtime_DeletedFilesInfo backingDeletedFilesInfo1 = {
    .deleted_files = 15,
    .deleted_bytes = 20,
  };
  constexpr _spoor_runtime_DeletedFilesInfo backingDeletedFilesInfo2 = {
    .deleted_files = 16,
    .deleted_bytes = 21,
  };
  SpoorDeletedFilesInfo *deletedFilesInfo1 = [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo1];
  SpoorDeletedFilesInfo *deletedFilesInfo2 = [[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:backingDeletedFilesInfo2];
  XCTAssertNotEqualObjects(deletedFilesInfo1, deletedFilesInfo2);
}

@end

#pragma GCC diagnostic pop
