// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import <UIKit/UIKit.h>

NS_ASSUME_NONNULL_BEGIN

@protocol OptionsViewControllerDelegate <NSObject>
- (void)didSelectBackgroundColor;
- (void)didSelectAnimationList;
- (void)didSelectKeyPaths;
- (void)didToggleEvents:(BOOL)enabled;
@end

@interface OptionsViewController : UIViewController

@property(nonatomic, weak) id<OptionsViewControllerDelegate> delegate;

@end

NS_ASSUME_NONNULL_END
