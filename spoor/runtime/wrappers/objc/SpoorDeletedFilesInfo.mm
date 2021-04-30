// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#import <Foundation/Foundation.h>
#import "SpoorDeletedFilesInfo_private.h"

@interface SpoorDeletedFilesInfo()

@property(assign, nonatomic) _spoor_runtime_DeletedFilesInfo deletedFilesInfo;

@end

@implementation SpoorDeletedFilesInfo : NSObject

-(instancetype)initWithDeletedFilesInfo:(_spoor_runtime_DeletedFilesInfo)deletedFilesInfo
{
  if (self = [super init])
  {
    self.deletedFilesInfo = deletedFilesInfo;
  }
  return self;
}

-(int32_t)deletedFiles
{
  return self.deletedFilesInfo.deleted_files;
}

-(int64_t)deletedBytes
{
  return self.deletedFilesInfo.deleted_bytes;
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
    const _spoor_runtime_DeletedFilesInfo& lhs = self.deletedFilesInfo;
    const _spoor_runtime_DeletedFilesInfo& rhs = ((SpoorDeletedFilesInfo *)object).deletedFilesInfo;
    return _spoor_runtime_DeletedFilesInfoEqual(&lhs, &rhs);
  }
}

@end
