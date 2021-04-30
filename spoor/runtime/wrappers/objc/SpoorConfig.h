// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorTypes.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorConfig : NSObject

@property(readonly) SpoorSizeType traceFilePathSize;
/** Non-owning */
@property(nullable, readonly) NSString *traceFilePath;
@property(readonly) SpoorSessionId sessionId;
@property(readonly) SpoorSizeType threadEventBufferCapacity;
@property(readonly) SpoorSizeType maxReservedEventBufferSliceCapacity;
@property(readonly) SpoorSizeType reservedEventPoolCapacity;
@property(readonly) SpoorSizeType dynamicEventPoolCapacity;
@property(readonly) SpoorSizeType dynamicEventSliceBorrowCASAttempts;
@property(readonly) SpoorDurationNanoseconds eventBufferRetentionDurationNanoseconds;
@property(readonly) int32_t maxFlushBufferToFileAttempts;
@property(readonly) bool flushAllEvents;

@end

NS_ASSUME_NONNULL_END
