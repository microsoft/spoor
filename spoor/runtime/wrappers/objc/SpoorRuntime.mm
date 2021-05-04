// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <functional>
#include <vector>
#include "spoor/runtime/runtime.h"

#import <Foundation/Foundation.h>
#import "SpoorConfig_private.h"
#import "SpoorDeletedFilesInfo_private.h"
#import "SpoorRuntime.h"

@implementation SpoorRuntime : NSObject

+ (void)initializeRuntime {
  _spoor_runtime_InitializeRuntime();
}

+ (void)deinitializeRuntime {
  _spoor_runtime_DeinitializeRuntime();
}

+ (BOOL)isRuntimeInitialized {
  return _spoor_runtime_RuntimeInitialized();
}

+ (void)enableRuntime {
  _spoor_runtime_EnableRuntime();
}

+ (void)disableRuntime {
  _spoor_runtime_DisableRuntime();
}

+ (BOOL)isRuntimeEnabled {
  return _spoor_runtime_RuntimeEnabled();
}

+ (void)logEvent:(const SpoorEventType)event
       timestamp:(const SpoorTimestampNanoseconds)steadyClockTimestamp
        payload1:(const uint64_t)payload1
        payload2:(const uint32_t)payload2 {
  _spoor_runtime_LogEventWithTimestamp(event, steadyClockTimestamp, payload1, payload2);
}

+ (void)logEvent:(const SpoorEventType)event
        payload1:(const uint64_t)payload1
        payload2:(const uint32_t)payload2 {
  _spoor_runtime_LogEvent(event, payload1, payload2);
}

+ (void)logFunctionEntryWithFunctionId:(const SpoorFunctionId)functionId {
  _spoor_runtime_LogFunctionEntry(functionId);
}

+ (void)logFunctionExitWithFunctionId:(const SpoorFunctionId)functionId {
  _spoor_runtime_LogFunctionExit(functionId);
}

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void()> flushTraceEventsWithCallback_{};

+ (void)flushTraceEventsWithCallback:(void (^)(void))callback {
  flushTraceEventsWithCallback_ = [callback]() {
    if (callback != nil) callback();
  };
  _spoor_runtime_FlushTraceEvents([]() { flushTraceEventsWithCallback_(); });
}

+ (void)clearTraceEvents {
  _spoor_runtime_ClearTraceEvents();
}

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(_spoor_runtime_TraceFiles)> flushedTraceFilesWithCallback_{};

+ (void)flushedTraceFilesWithCallback:(void (^)(const NSArray<NSString*>* _Nullable))callback {
  flushedTraceFilesWithCallback_ = [callback](_spoor_runtime_TraceFiles traceFiles) {
    NSMutableArray<NSString*>* filePaths;
    if (traceFiles.file_paths != nil) {
      filePaths = [NSMutableArray arrayWithCapacity:traceFiles.file_paths_size];
      for (size_t i = 0; i < traceFiles.file_paths_size; ++i) {
        char* charFilePath = traceFiles.file_paths[i];
        [filePaths addObject:[[NSString alloc] initWithUTF8String:charFilePath]];
      }
      _spoor_runtime_ReleaseTraceFilePaths(&traceFiles);
    }

    if (callback != nil) {
      callback(static_cast<const NSArray<NSString*>*>(filePaths));
    }
  };

  _spoor_runtime_FlushedTraceFiles(
      [](_spoor_runtime_TraceFiles traceFiles) { flushedTraceFilesWithCallback_(traceFiles); });
}

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(_spoor_runtime_DeletedFilesInfo)>
    deleteFlushedTraceFilesOlderThanTimestampCallback_{};

+ (void)deleteFlushedTraceFilesOlderThanDate:(const NSDate*)date
                                    callback:(void (^)(const SpoorDeletedFilesInfo*))callback {
  deleteFlushedTraceFilesOlderThanTimestampCallback_ =
      [callback](_spoor_runtime_DeletedFilesInfo deletedFilesInfo) {
        if (callback != nil)
          callback([[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:deletedFilesInfo]);
      };
  _spoor_runtime_SystemTimestampSeconds systemTimestamp = date.timeIntervalSince1970;
  _spoor_runtime_DeleteFlushedTraceFilesOlderThan(
      systemTimestamp, [](_spoor_runtime_DeletedFilesInfo deletedFilesInfo) {
        deleteFlushedTraceFilesOlderThanTimestampCallback_(deletedFilesInfo);
      });
}

+ (SpoorConfig*)config {
  _spoor_runtime_Config config = _spoor_runtime_GetConfig();
  return [[SpoorConfig alloc] initWithConfig:config];
}

+ (BOOL)isStubImplementation {
  return _spoor_runtime_StubImplementation();
}

@end
