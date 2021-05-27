// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorConfig.h"

#include "spoor/runtime/runtime.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorConfig (Private)

@property(readonly, assign, nonatomic) spoor::runtime::Config config;

- (instancetype)initWithConfig:(spoor::runtime::Config)config;

@end

NS_ASSUME_NONNULL_END
