// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SpoorConfig.h"

NS_ASSUME_NONNULL_BEGIN

@interface SpoorConfig (Private)

@property(readonly, assign, nonatomic) _spoor_runtime_Config config;

- (instancetype)initWithConfig:(_spoor_runtime_Config)config;

@end

NS_ASSUME_NONNULL_END
