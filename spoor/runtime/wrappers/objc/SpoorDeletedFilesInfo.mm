// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#import <Foundation/Foundation.h>
#import "SpoorDeletedFilesInfo_private.h"

@interface SpoorDeletedFilesInfo ()

@property(assign, nonatomic) _spoor_runtime_DeletedFilesInfo deletedFilesInfo;

@end

@implementation SpoorDeletedFilesInfo : NSObject

- (instancetype)initWithDeletedFilesInfo:(_spoor_runtime_DeletedFilesInfo)deletedFilesInfo {
  self = [super init];
  if (self != nil) {
    self.deletedFilesInfo = deletedFilesInfo;
  }
  return self;
}

- (int32_t)deletedFiles {
  return self.deletedFilesInfo.deleted_files;
}

- (int64_t)deletedBytes {
  return self.deletedFilesInfo.deleted_bytes;
}

- (BOOL)isEqual:(id)object {
  if (object == self) {
    return YES;
  } else if (![object isKindOfClass:[self class]]) {
    return NO;
  } else {
    auto lhs = self.deletedFilesInfo;
    auto rhs = static_cast<SpoorDeletedFilesInfo*>(object).deletedFilesInfo;
    return _spoor_runtime_DeletedFilesInfoEqual(&lhs, &rhs);
  }
}

@end
