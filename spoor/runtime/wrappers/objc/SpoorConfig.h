// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <stdint.h>

#import <Foundation/Foundation.h>
#import "SpoorTypes.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorConfig : NSObject

@property(nullable, readonly) NSString* traceFilePath;
@property(readonly) SpoorSessionId sessionId;
@property(readonly) SpoorSizeType threadEventBufferCapacity;
@property(readonly) SpoorSizeType maxReservedEventBufferSliceCapacity;
@property(readonly) SpoorSizeType maxDynamicEventBufferSliceCapacity;
@property(readonly) SpoorSizeType reservedEventPoolCapacity;
@property(readonly) SpoorSizeType dynamicEventPoolCapacity;
@property(readonly) SpoorSizeType dynamicEventSliceBorrowCASAttempts;
@property(readonly) SpoorDurationNanoseconds eventBufferRetentionDuration;
@property(readonly) int32_t maxFlushBufferToFileAttempts;
@property(readonly) BOOL flushAllEvents;

@end

NS_ASSUME_NONNULL_END
