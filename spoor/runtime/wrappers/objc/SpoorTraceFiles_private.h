// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorTraceFiles.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorTraceFiles (Private)

@property(readonly, assign, nonatomic) _spoor_runtime_TraceFiles traceFiles;

-(instancetype)initWithTraceFiles:(_spoor_runtime_TraceFiles)traceFiles;

@end

NS_ASSUME_NONNULL_END
