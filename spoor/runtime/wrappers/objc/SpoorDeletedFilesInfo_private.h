// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorDeletedFilesInfo.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorDeletedFilesInfo (Private)

@property(readonly, assign, nonatomic) _spoor_runtime_DeletedFilesInfo deletedFilesInfo;

- (instancetype)initWithDeletedFilesInfo:(_spoor_runtime_DeletedFilesInfo)deletedFilesInfo;

@end

NS_ASSUME_NONNULL_END
