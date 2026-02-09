// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@protocol SideMenuDelegate <NSObject>
- (void)didSelectPropertyUpdate;
- (void)didSelectPropertyCallback;
- (void)didSelectMultiAnimation;
@end

@interface SideMenuViewController : UIViewController

@property(nonatomic, weak) id<SideMenuDelegate> delegate;

- (void)showInViewController:(UIViewController *)viewController;
- (void)dismiss;

@end

NS_ASSUME_NONNULL_END
