// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#import "SceneDelegate.h"
#import "ViewController.h"

@interface SceneDelegate ()
@end

@implementation SceneDelegate

- (void)scene:(UIScene*)scene
    willConnectToSession:(UISceneSession*)session
                 options:(UISceneConnectionOptions*)connectionOptions {
  if (![scene isKindOfClass:[UIWindowScene class]]) return;
  UIWindowScene* windowScene = (UIWindowScene*)scene;
  UIWindow* window = [[UIWindow alloc] initWithWindowScene:windowScene];
  ViewController* viewController = [[ViewController alloc] init];
  [window setRootViewController:viewController];
  [window makeKeyAndVisible];
  self.window = window;
}

@end
