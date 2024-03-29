// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "Runtime.h"

#import <Foundation/Foundation.h>

#include <functional>
#include <vector>

#include "spoor/runtime/runtime.h"

#import "SpoorConfig_private.h"
#import "SpoorDeletedFilesInfo_private.h"

@implementation SpoorRuntime : NSObject

+ (void)setUp {
  spoor::runtime::Initialize();
}

+ (void)tearDown {
  spoor::runtime::Deinitialize();
}

+ (BOOL)isSetUp {
  return spoor::runtime::Initialized();
}

+ (void)enableLogging {
  spoor::runtime::Enable();
}

+ (void)disableLogging {
  spoor::runtime::Disable();
}

+ (BOOL)isLoggingEnabled {
  return spoor::runtime::Enabled();
}

+ (void)logEvent:(const SpoorEventType)event
       timestamp:(const SpoorTimestampNanoseconds)steadyClockTimestamp
        payload1:(const uint64_t)payload1
        payload2:(const uint32_t)payload2 {
  spoor::runtime::LogEvent(event, steadyClockTimestamp, payload1, payload2);
}

+ (void)logEvent:(const SpoorEventType)event
        payload1:(const uint64_t)payload1
        payload2:(const uint32_t)payload2 {
  spoor::runtime::LogEvent(event, payload1, payload2);
}

+ (void)flushTraceEventsWithCallback:(void (^)(void))callback {
  spoor::runtime::FlushTraceEvents([callback]() {
    if (callback != nil) callback();
  });
}

+ (void)clearTraceEvents {
  spoor::runtime::ClearTraceEvents();
}

+ (void)flushedTraceFilesWithCallback:(void (^)(const NSArray<NSString*>*))callback {
  spoor::runtime::FlushedTraceFiles([callback](std::vector<std::filesystem::path> paths) {
    NSMutableArray<NSString*>* convertedPaths = [NSMutableArray arrayWithCapacity:paths.size()];
    for (auto& path : paths) {
      [convertedPaths addObject:[[NSString alloc] initWithUTF8String:path.c_str()]];
    }

    if (callback != nil) {
      callback(static_cast<const NSArray<NSString*>*>(convertedPaths));
    }
  });
}

+ (void)deleteFlushedTraceFilesOlderThanDate:(const NSDate*)date
                                    callback:(void (^)(const SpoorDeletedFilesInfo*))callback {
  spoor::runtime::SystemTimestampSeconds systemTimestamp = date.timeIntervalSince1970;
  spoor::runtime::DeleteFlushedTraceFilesOlderThan(
      systemTimestamp, [callback](spoor::runtime::DeletedFilesInfo deletedFilesInfo) {
        if (callback != nil) {
          callback([[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:deletedFilesInfo]);
        }
      });
}

+ (const SpoorConfig*)config {
  spoor::runtime::Config config = spoor::runtime::GetConfig();
  return [[SpoorConfig alloc] initWithConfig:config];
}

+ (BOOL)isStubImplementation {
  return spoor::runtime::StubImplementation();
}

@end
