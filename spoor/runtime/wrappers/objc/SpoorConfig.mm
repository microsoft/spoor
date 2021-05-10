// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorConfig.h"

#import <Foundation/Foundation.h>

#include "spoor/runtime/runtime.h"

@interface SpoorConfig ()

@property(readwrite, assign, nonatomic) spoor::runtime::Config config;

@end

@implementation SpoorConfig : NSObject

- (instancetype)initWithConfig:(spoor::runtime::Config)config {
  self = [super init];
  if (self != nil) {
    self.config = std::move(config);
  }
  return self;
}

- (const NSString*)traceFilePath {
  if (self.config.trace_file_path.empty()) {
    return nil;
  }

  return [[NSString alloc] initWithUTF8String:self.config.trace_file_path.c_str()];
}

- (SpoorSessionId)sessionId {
  return self.config.session_id;
}

- (SpoorSizeType)threadEventBufferCapacity {
  return self.config.thread_event_buffer_capacity;
}

- (SpoorSizeType)maxReservedEventBufferSliceCapacity {
  return self.config.max_reserved_event_buffer_slice_capacity;
}

- (SpoorSizeType)maxDynamicEventBufferSliceCapacity {
  return self.config.max_dynamic_event_buffer_slice_capacity;
}

- (SpoorSizeType)reservedEventPoolCapacity {
  return self.config.reserved_event_pool_capacity;
}

- (SpoorSizeType)dynamicEventPoolCapacity {
  return self.config.dynamic_event_pool_capacity;
}

- (SpoorSizeType)dynamicEventSliceBorrowCASAttempts {
  return self.config.dynamic_event_slice_borrow_cas_attempts;
}

- (SpoorDurationNanoseconds)eventBufferRetentionDuration {
  return self.config.event_buffer_retention_duration_nanoseconds;
}

- (int32_t)maxFlushBufferToFileAttempts {
  return self.config.max_flush_buffer_to_file_attempts;
}

- (BOOL)flushAllEvents {
  return self.config.flush_all_events;
}

- (BOOL)isEqual:(id)object {
  if (object == self) {
    return YES;
  } else if (![object isKindOfClass:[self class]]) {
    return NO;
  } else {
    const auto lhs = self.config;
    const auto rhs = static_cast<SpoorConfig*>(object).config;
    return lhs == rhs;
  }
}

@end
