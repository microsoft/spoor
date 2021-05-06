// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "spoor/runtime/runtime.h"

#import "SpoorConfig.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorConfig (Private)

@property(readonly, assign, nonatomic) spoor::runtime::Config config;

- (instancetype)initWithConfig:(spoor::runtime::Config)config;

@end

NS_ASSUME_NONNULL_END
