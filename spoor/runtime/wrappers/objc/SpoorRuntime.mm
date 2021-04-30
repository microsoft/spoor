// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include <functional>
#include <vector>
#include "spoor/runtime/runtime.h"

#import <Foundation/Foundation.h>
#import "SpoorRuntime.h"
#import "SpoorConfig_private.h"
#import "SpoorTraceFiles_private.h"
#import "SpoorDeletedFilesInfo_private.h"

@implementation SpoorRuntime : NSObject

+(void)initializeRuntime
{
  _spoor_runtime_InitializeRuntime();
}

+(void)deinitializeRuntime
{
  _spoor_runtime_DeinitializeRuntime();
}

+(bool)runtimeInitialized
{
  return _spoor_runtime_RuntimeInitialized();
}

+(void)enableRuntime
{
  _spoor_runtime_EnableRuntime();
}

+(void)disableRuntime
{
  _spoor_runtime_DisableRuntime();
}

+(bool)runtimeEnabled
{
  return _spoor_runtime_RuntimeEnabled();
}

+(void)logEvent:(SpoorEventType)event
      timestamp:(SpoorTimestampNanoseconds)steadyClockTimestamp
       payload1:(uint64_t)payload1
       payload2:(uint32_t)payload2
{
  _spoor_runtime_LogEventWithTimestamp(event, steadyClockTimestamp, payload1, payload2);
}

+(void)logEvent:(SpoorEventType)event
       payload1:(uint64_t)payload1
       payload2:(uint32_t)payload2
{
  _spoor_runtime_LogEvent(event, payload1, payload2);
}

+(void)logFunctionEntryWithFunctionId:(SpoorFunctionId)functionId
{
  _spoor_runtime_LogFunctionEntry(functionId);
}

+(void)logFunctionExitWithFunctionId:(SpoorFunctionId)functionId
{
  _spoor_runtime_LogFunctionExit(functionId);
}

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void()> flushTraceEventsWithCallback_{};

+(void)flushTraceEventsWithCallback:(void (^)(void) _Nullable)callback
{
  flushTraceEventsWithCallback_ = [callback]() { if (callback) callback(); };
  _spoor_runtime_FlushTraceEvents([]() { flushTraceEventsWithCallback_(); });
}

+(void)clearTraceEvents
{
  _spoor_runtime_ClearTraceEvents();
}

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(_spoor_runtime_TraceFiles)> flushedTraceFilesWithCallback_{};

+(void)flushedTraceFilesWithCallback:(void (^)(SpoorTraceFiles *) _Nullable)callback
{
  flushedTraceFilesWithCallback_ = [callback](_spoor_runtime_TraceFiles traceFiles) {
    if (callback) callback([[SpoorTraceFiles alloc] initWithTraceFiles:traceFiles]);
  };
  _spoor_runtime_FlushedTraceFiles([](_spoor_runtime_TraceFiles traceFiles) {
    flushedTraceFilesWithCallback_(traceFiles);
  });
}

// clang-format off NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables, fuchsia-statically-constructed-objects) clang-format on
std::function<void(_spoor_runtime_DeletedFilesInfo)> deleteFlushedTraceFilesOlderThanTimestampCallback_{};

+(void)deleteFlushedTraceFilesOlderThanTimestamp:(SpoorSystemTimestampSeconds)systemTimestamp
                                        callback:(void (^)(SpoorDeletedFilesInfo *) _Nullable)callback
{
  deleteFlushedTraceFilesOlderThanTimestampCallback_ = [callback](_spoor_runtime_DeletedFilesInfo deletedFilesInfo) {
    if (callback) callback([[SpoorDeletedFilesInfo alloc] initWithDeletedFilesInfo:deletedFilesInfo]);
  };
  _spoor_runtime_DeleteFlushedTraceFilesOlderThan(systemTimestamp, [](_spoor_runtime_DeletedFilesInfo deletedFilesInfo) {
    deleteFlushedTraceFilesOlderThanTimestampCallback_(deletedFilesInfo);
  });
}

+(SpoorConfig *)config
{
  _spoor_runtime_Config config = _spoor_runtime_GetConfig();
  return [[SpoorConfig alloc] initWithConfig:config];
}

+(bool)stubImplementation
{
  return _spoor_runtime_StubImplementation();
}

+(void)releaseTraceFilePaths:(NSArray<SpoorTraceFiles *> *)traceFilePaths
{
  std::vector<_spoor_runtime_TraceFiles> convertedTraceFilePaths(traceFilePaths.count);
  for (size_t i = 0; i < traceFilePaths.count; ++i)
  {
    convertedTraceFilePaths[i] = traceFilePaths[i].traceFiles;
  }

  _spoor_runtime_ReleaseTraceFilePaths(convertedTraceFilePaths.data());
}

@end
