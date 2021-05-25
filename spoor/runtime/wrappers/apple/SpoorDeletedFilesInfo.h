// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import <Foundation/Foundation.h>

#include <stdint.h>

NS_ASSUME_NONNULL_BEGIN

@interface SpoorDeletedFilesInfo : NSObject

@property(readonly) int32_t deletedFiles;
@property(readonly) int64_t deletedBytes;

@end

NS_ASSUME_NONNULL_END
