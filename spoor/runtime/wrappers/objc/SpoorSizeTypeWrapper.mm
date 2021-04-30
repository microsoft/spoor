// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import <Foundation/Foundation.h>
#import "SpoorSizeTypeWrapper.h"

@interface SpoorSizeTypeWrapper()

@property(assign, nonatomic) SpoorSizeType sizeType;

@end

@implementation SpoorSizeTypeWrapper : NSObject

-(instancetype)initWithSizeType:(SpoorSizeType)sizeType
{
  if (self = [super init])
  {
    self.sizeType = sizeType;
  }
  return self;
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
    return self.sizeType == ((SpoorSizeTypeWrapper *)object).sizeType;
  }
}

@end
