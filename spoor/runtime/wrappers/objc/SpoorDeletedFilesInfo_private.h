// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorDeletedFilesInfo.h"

#include "spoor/runtime/runtime.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorDeletedFilesInfo (Private)

@property(readonly, assign, nonatomic) spoor::runtime::DeletedFilesInfo deletedFilesInfo;

- (instancetype)initWithDeletedFilesInfo:(spoor::runtime::DeletedFilesInfo)deletedFilesInfo;

@end

NS_ASSUME_NONNULL_END
