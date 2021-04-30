// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#import <Foundation/Foundation.h>
#import "SpoorTraceFiles.h"
#import "SpoorSizeTypeWrapper_private.h"

@interface SpoorTraceFiles()

@property(readwrite, assign, nonatomic) _spoor_runtime_TraceFiles traceFiles;

@end

@implementation SpoorTraceFiles : NSObject

-(instancetype)initWithTraceFiles:(_spoor_runtime_TraceFiles)traceFiles
{
  if (self = [super init])
  {
    self.traceFiles = traceFiles;
  }
  return self;
}

-(SpoorSizeType)filePathsSize
{
  return self.traceFiles.file_paths_size;
}

-(NSArray<SpoorSizeTypeWrapper *> * _Nullable)filePathSizes
{
  NSMutableArray<SpoorSizeTypeWrapper *> *filePathSizes;
  if (self.traceFiles.file_path_sizes != nil)
  {
    filePathSizes = [NSMutableArray arrayWithCapacity:self.traceFiles.file_paths_size];
    for (size_t i = 0; i < self.traceFiles.file_paths_size; ++i)
    {
      _spoor_runtime_SizeType sizeType = self.traceFiles.file_path_sizes[i];
      [filePathSizes addObject:[[SpoorSizeTypeWrapper alloc] initWithSizeType:sizeType]];
    }
  }
  return filePathSizes;
}

-(NSArray<NSString *> * _Nullable)filePaths
{
  NSMutableArray<NSString *> *filePaths;
  if (self.traceFiles.file_paths != nil)
  {
    filePaths = [NSMutableArray arrayWithCapacity:self.traceFiles.file_paths_size];
    for (size_t i = 0; i < self.traceFiles.file_paths_size; ++i)
    {
      char *charFilePath = self.traceFiles.file_paths[i];
      [filePaths addObject:[NSString stringWithUTF8String:charFilePath]];
    }
  }
  return filePaths;
}

-(BOOL)isEqual:(id)object
{
  if (object == self)
  {
    return YES;
  }
  else if (!object || ![object isKindOfClass:[self class]])
  {
    return NO;
  }
  else
  {
    const _spoor_runtime_TraceFiles& lhs = self.traceFiles;
    const _spoor_runtime_TraceFiles& rhs = ((SpoorTraceFiles *)object).traceFiles;
    return _spoor_runtime_TraceFilesEqual(&lhs, &rhs);
  }
}

@end
