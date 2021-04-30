// Copyright (c)Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#import <Foundation/Foundation.h>
#import "SpoorConfig.h"

@interface SpoorConfig()

@property(readwrite, assign, nonatomic) _spoor_runtime_Config config;

@end

@implementation SpoorConfig : NSObject

-(instancetype)initWithConfig:(_spoor_runtime_Config)config
{
  if (self = [super init])
  {
    self.config = config;
  }
  return self;
}

-(SpoorSizeType)traceFilePathSize
{
  return self.config.trace_file_path_size;
}

-(NSString * _Nullable)traceFilePath
{
  if (self.config.trace_file_path == nil)
  {
    return nil;
  }

  return [NSString stringWithUTF8String:self.config.trace_file_path];
}

-(SpoorSessionId)sessionId
{
  return self.config.session_id;
}

-(SpoorSizeType)threadEventBufferCapacity
{
  return self.config.thread_event_buffer_capacity;
}

-(SpoorSizeType)maxReservedEventBufferSliceCapacity
{
  return self.config.max_reserved_event_buffer_slice_capacity;
}

-(SpoorSizeType)reservedEventPoolCapacity
{
  return self.config.reserved_event_pool_capacity;
}

-(SpoorSizeType)dynamicEventPoolCapacity
{
  return self.config.dynamic_event_pool_capacity;
}

-(SpoorSizeType)dynamicEventSliceBorrowCASAttempts
{
  return self.config.dynamic_event_slice_borrow_cas_attempts;
}

-(SpoorDurationNanoseconds)eventBufferRetentionDurationNanoseconds
{
  return self.config.event_buffer_retention_duration_nanoseconds;
}

-(int32_t)maxFlushBufferToFileAttempts
{
  return self.config.max_flush_buffer_to_file_attempts;
}

-(bool)flushAllEvents
{
  return self.config.flush_all_events;
}

-(BOOL)isEqual:(id)object
{
  if (object == self)
  {
    return YES;
  }
  else if (!object || ![object isKindOfClass:[self class]])
  {
    return NO;
  }
  else
  {
    const _spoor_runtime_Config& lhs = self.config;
    const _spoor_runtime_Config& rhs = ((SpoorConfig *)object).config;
    return _spoor_runtime_ConfigEqual(&lhs, &rhs);
  }
}

@end
