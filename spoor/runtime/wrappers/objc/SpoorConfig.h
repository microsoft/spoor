// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import <Foundation/Foundation.h>

#include <stdint.h>

#import "SpoorTypes.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorConfig : NSObject

@property(nullable, readonly) const NSString* traceFilePath;
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
