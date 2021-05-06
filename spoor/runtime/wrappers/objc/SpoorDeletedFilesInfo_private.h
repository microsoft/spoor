// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#import "SpoorDeletedFilesInfo.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorDeletedFilesInfo (Private)

@property(readonly, assign, nonatomic) spoor::runtime::DeletedFilesInfo deletedFilesInfo;

- (instancetype)initWithDeletedFilesInfo:(spoor::runtime::DeletedFilesInfo)deletedFilesInfo;

@end

NS_ASSUME_NONNULL_END
