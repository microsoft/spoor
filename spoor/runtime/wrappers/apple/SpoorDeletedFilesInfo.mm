// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorDeletedFilesInfo_private.h"

#import <Foundation/Foundation.h>

#include "spoor/runtime/runtime.h"

@interface SpoorDeletedFilesInfo ()

@property(assign, nonatomic) spoor::runtime::DeletedFilesInfo deletedFilesInfo;

@end

@implementation SpoorDeletedFilesInfo : NSObject

- (instancetype)initWithDeletedFilesInfo:(spoor::runtime::DeletedFilesInfo)deletedFilesInfo {
  self = [super init];
  if (self != nil) {
    self.deletedFilesInfo = std::move(deletedFilesInfo);
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
    const auto lhs = self.deletedFilesInfo;
    const auto rhs = static_cast<SpoorDeletedFilesInfo*>(object).deletedFilesInfo;
    return lhs == rhs;
  }
}

@end
