// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorConfig.h"
#import "SpoorDeletedFilesInfo.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorRuntime : NSObject

/**
 Initialize the event tracing runtime. A call to this function is inserted at
 the start of `main` by the compile-time instrumentation unless configured
 otherwise.
 */
+ (void)initializeRuntime;

/**
 Deinitialize the event tracing runtime. A call to this function is inserted
 at the  end of `main` by the compile-time instrumentation.
 */
+ (void)deinitializeRuntime;

/**
 Check if the event tracing runtime is initialized.
 */
+ (BOOL)isRuntimeInitialized;

/**
 Enable runtime logging.
 */
+ (void)enableRuntime;

/**
 Disable runtime logging.
 */
+ (void)disableRuntime;

/**
 Check if runtime logging is enabled.
 */
+ (BOOL)isRuntimeEnabled;

/**
 Log that the program generated an event. The function internally checks if
 the runtime is enabled before logging the event.
 */
+ (void)logEvent:(SpoorEventType)event
       timestamp:(SpoorTimestampNanoseconds)steadyClockTimestamp
        payload1:(uint64_t)payload1
        payload2:(uint32_t)payload2;

/**
 Log that the program generated an event. The function internally checks if
 the runtime is enabled before collecting the current timestamp and logging
 the event.
 */
+ (void)logEvent:(SpoorEventType)event payload1:(uint64_t)payload1 payload2:(uint32_t)payload2;

/**
 Log that the program entered a function. The function internally checks if
 the runtime is enabled before collecting the current timestamp and logging
 the event. A call to this function is inserted at the start of every function
 by the compile-time instrumentation.
 */
+ (void)logFunctionEntryWithFunctionId:(SpoorFunctionId)functionId;

/**
 Log that the program exited a function. The function internally checks if the
 runtime is enabled before collecting the current timestamp and logging the
 event. A call to this function is inserted at the end of every function by
 the compile-time instrumentation.
 */
+ (void)logFunctionExitWithFunctionId:(SpoorFunctionId)functionId;

/**
 Flush in-memory trace events to disk.
 */
+ (void)flushTraceEventsWithCallback:(void (^)(void))callback;

/**
 Clear the trace events from memory without flushing them to disk.
 */
+ (void)clearTraceEvents;

/**
 Retrieve an array of all trace files on disk.
 */
+ (void)flushedTraceFilesWithCallback:(void (^)(const NSArray<NSString*>* _Nullable))callback;

/**
 Delete all trace files older than a given date.
 */
+ (void)deleteFlushedTraceFilesOlderThanDate:(const NSDate*)date
                                    callback:(void (^)(const SpoorDeletedFilesInfo*))callback;

/**
 Retrieve Spoor's configuration.
 */
+ (SpoorConfig*)config;

/**
 Check if the runtime contains stub implementations.
 */
+ (BOOL)isStubImplementation;

@end

NS_ASSUME_NONNULL_END
