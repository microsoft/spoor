// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorConfig.h"
#import "SpoorDeletedFilesInfo.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorRuntime : NSObject

/**
 Set up the event tracing runtime. A call to this function is inserted at
 the start of `main` by the compile-time instrumentation unless configured
 otherwise.
 */
+ (void)setUp;

/**
 Tear down the event tracing runtime. A call to this function is inserted
 at the end of `main` by the compile-time instrumentation.
 */
+ (void)tearDown;

/**
 Check if the event tracing runtime is set up.
 */
+ (BOOL)isSetUp;

/**
 Enable runtime logging.
 */
+ (void)enableLogging;

/**
 Disable runtime logging.
 */
+ (void)disableLogging;

/**
 Check if runtime logging is enabled.
 */
+ (BOOL)isLoggingEnabled;

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
+ (void)flushedTraceFilesWithCallback:(void (^)(const NSArray<NSString*>*))callback;

/**
 Delete all trace files older than a given date.
 */
+ (void)deleteFlushedTraceFilesOlderThanDate:(const NSDate*)date
                                    callback:(void (^)(const SpoorDeletedFilesInfo*))callback;

/**
 Retrieve Spoor's configuration.
 */
+ (const SpoorConfig*)config;

/**
 Check if the runtime contains stub implementations.
 */
+ (BOOL)isStubImplementation;

@end

NS_ASSUME_NONNULL_END
