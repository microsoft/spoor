// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wgnu-statement-expression"

#include "spoor/runtime/runtime.h"
#include <vector>

#import <XCTest/XCTest.h>
#import "SpoorTraceFiles_private.h"
#import "SpoorSizeTypeWrapper_private.h"

@interface SpoorTraceFilesTests : XCTestCase

@end

@implementation SpoorTraceFilesTests

-(void)testFilePathsSize
{
  constexpr _spoor_runtime_TraceFiles backingTraceFiles{
    .file_paths_size = 6};
  SpoorTraceFiles *traceFiles = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles];
  XCTAssertEqual(traceFiles.filePathsSize, (SpoorSizeType)6);
}

-(void)testFilePathSizes
{
  _spoor_runtime_SizeType filePathSizes[] = {
    (_spoor_runtime_SizeType)5,
    (_spoor_runtime_SizeType)8,
    (_spoor_runtime_SizeType)1,
    (_spoor_runtime_SizeType)100,
  };
  _spoor_runtime_TraceFiles backingTraceFiles{
    .file_paths_size = 4,
    .file_path_sizes = filePathSizes};
  SpoorTraceFiles *traceFiles = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles];
  NSArray<SpoorSizeTypeWrapper *> *expectedFilePathSizes = @[
    [[SpoorSizeTypeWrapper alloc] initWithSizeType:(SpoorSizeType)5],
    [[SpoorSizeTypeWrapper alloc] initWithSizeType:(SpoorSizeType)8],
    [[SpoorSizeTypeWrapper alloc] initWithSizeType:(SpoorSizeType)1],
    [[SpoorSizeTypeWrapper alloc] initWithSizeType:(SpoorSizeType)100],
  ];
  XCTAssertEqualObjects(traceFiles.filePathSizes, expectedFilePathSizes);
}

-(void)testFilePathSizesNil
{
  _spoor_runtime_TraceFiles backingTraceFiles{
    .file_path_sizes = nil};
  SpoorTraceFiles *traceFiles = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles];
  XCTAssertNil(traceFiles.filePathSizes);
}

-(void)testFilePathSizesEmpty
{
  _spoor_runtime_TraceFiles backingTraceFiles{
    .file_paths_size = 0,
    .file_path_sizes = {}};
  SpoorTraceFiles *traceFiles = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles];
  XCTAssertEqual(traceFiles.filePathSizes.count, (unsigned long)0);
}

-(void)testFilePaths
{
  char *filePaths[5] = {
    (char*)"first/path",
    (char*)"second/path",
    (char*)"third/path",
    (char*)"fourth/path",
    (char*)"fifth/path",
  };
  _spoor_runtime_TraceFiles backingTraceFiles{
    .file_paths_size = 5,
    .file_paths = filePaths};
  SpoorTraceFiles *traceFiles = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles];
  NSArray<NSString *> *expectedFilePaths = @[
    @"first/path",
    @"second/path",
    @"third/path",
    @"fourth/path",
    @"fifth/path",
  ];
  XCTAssertEqualObjects(traceFiles.filePaths, expectedFilePaths);
}

-(void)testFilePathsNil
{
  _spoor_runtime_TraceFiles backingTraceFiles{
    .file_paths = nil};
  SpoorTraceFiles *traceFiles = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles];
  XCTAssertNil(traceFiles.filePaths);
}

-(void)testFilePathsEmpty
{
  _spoor_runtime_TraceFiles backingTraceFiles{
    .file_paths_size = 0,
    .file_paths = {}};
  SpoorTraceFiles *traceFiles = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles];
  XCTAssertEqual(traceFiles.filePaths.count, (unsigned long)0);
}

-(void)testEqualSameObjects
{
  _spoor_runtime_SizeType filePathSizes[] = {
    (_spoor_runtime_SizeType)10,
    (_spoor_runtime_SizeType)11,
    (_spoor_runtime_SizeType)10,
  };
  char *filePaths[3] = {
    (char*)"first/path",
    (char*)"second/path",
    (char*)"third/path",
  };
  _spoor_runtime_TraceFiles backingTraceFiles{
    .file_paths_size = 3,
    .file_path_sizes = filePathSizes,
    .file_paths = filePaths};
  SpoorTraceFiles *traceFiles1 = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles];
  SpoorTraceFiles *traceFiles2 = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles];
  XCTAssertEqualObjects(traceFiles1, traceFiles2);
}

-(void)testEqualDifferentObjects
{
  _spoor_runtime_SizeType filePathSizes1[] = {
    (_spoor_runtime_SizeType)10,
    (_spoor_runtime_SizeType)11,
    (_spoor_runtime_SizeType)10,
  };
  char *filePaths1[3] = {
    (char*)"first/path",
    (char*)"second/path",
    (char*)"third/path",
  };
  _spoor_runtime_TraceFiles backingTraceFiles1{
    .file_paths_size = 3,
    .file_path_sizes = filePathSizes1,
    .file_paths = filePaths1};

  _spoor_runtime_SizeType filePathSizes2[] = {
    (_spoor_runtime_SizeType)10,
    (_spoor_runtime_SizeType)11,
    (_spoor_runtime_SizeType)10,
  };
  char *filePaths2[3] = {
    (char*)"first/path",
    (char*)"second/path",
    (char*)"third/path",
  };
  _spoor_runtime_TraceFiles backingTraceFiles2{
    .file_paths_size = 3,
    .file_path_sizes = filePathSizes2,
    .file_paths = filePaths2};

  SpoorTraceFiles *traceFiles1 = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles1];
  SpoorTraceFiles *traceFiles2 = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles2];
  XCTAssertEqualObjects(traceFiles1, traceFiles2);
}

-(void)testNotEqual
{
  _spoor_runtime_SizeType filePathSizes1[] = {
    (_spoor_runtime_SizeType)10,
    (_spoor_runtime_SizeType)11,
    (_spoor_runtime_SizeType)10,
  };
  char *filePaths1[3] = {
    (char*)"first/path",
    (char*)"second/path",
    (char*)"third/path",
  };
  _spoor_runtime_TraceFiles backingTraceFiles1{
    .file_paths_size = 3,
    .file_path_sizes = filePathSizes1,
    .file_paths = filePaths1};

  _spoor_runtime_SizeType filePathSizes2[] = {
    (_spoor_runtime_SizeType)10,
    (_spoor_runtime_SizeType)11,
    (_spoor_runtime_SizeType)10u,
  };
  char *filePaths2[3] = {
    (char*)"first/path",
    (char*)"second/path",
    (char*)"thirf/path",
  };
  _spoor_runtime_TraceFiles backingTraceFiles2{
    .file_paths_size = 3,
    .file_path_sizes = filePathSizes2,
    .file_paths = filePaths2};

  SpoorTraceFiles *traceFiles1 = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles1];
  SpoorTraceFiles *traceFiles2 = [[SpoorTraceFiles alloc] initWithTraceFiles:backingTraceFiles2];
  XCTAssertNotEqualObjects(traceFiles1, traceFiles2);
}

@end

#pragma GCC diagnostic pop
