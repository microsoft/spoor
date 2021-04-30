// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorSizeTypeWrapper.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorTraceFiles : NSObject

@property(readonly) SpoorSizeType filePathsSize;
/**
 NOTE: The caller takes ownership of `file_path_sizes` and `file_paths`, and
 is responsible for releasing the memory, e.g., by calling
 `releaseTraceFilePaths`.
 */
@property(nullable, readonly) NSArray<SpoorSizeTypeWrapper *> *filePathSizes;
@property(nullable, readonly) NSArray<NSString *> *filePaths;

@end

NS_ASSUME_NONNULL_END
