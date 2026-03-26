// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "ViewController.h"
#import <AnimaX/AnimaXCompositionFactory.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <LottieAdapter/LottieAdapterAnimaXCreator.h>
#import <LottieAdapter/LottieAdapterFactory.h>
#import <LottieAdapter/LottieAdapterLottieCreator.h>
#import <LottieAdapter/LottieAdapterView.h>
#import <UIKit/UIKit.h>
#import "EmptyViewController.h"
#import "LottieAdapterViewController.h"
#import "PropertyCallbackController.h"
#import "PropertyUpdateController.h"

@interface ViewController ()
@property(nonatomic, strong) UILabel *titleLabel;
@property(nonatomic, strong) LottieAdapterView *animaXView;
@property(nonatomic, strong) LottieAdapterView *lottieView;
@property(nonatomic, strong) UIStackView *buttonStackView;
@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.view.backgroundColor = [UIColor whiteColor];

  [self addTitleLabel];
  [self setupButtons];

  [LottieAdapterFactory registerCreator:[[LottieAdapterAnimaXCreator alloc] init]
                                forType:AnimationTypeAnimaX];
  [LottieAdapterFactory registerCreator:[[LottieAdapterLottieCreator alloc] init]
                                forType:AnimationTypeLottie];

  [self loadAnimationWithName:@"animation_1" bundleName:nil];
}

- (void)loadAnimationWithName:(NSString *)animationName bundleName:(NSString *)bundleName {
  // AnimaX
  [LottieAdapterFactory setDefaultType:AnimationTypeAnimaX];
  [self newAnimatedViewWithType:AnimationTypeAnimaX
                  animationName:animationName
                     bundleName:bundleName];

  // Lottie
  [LottieAdapterFactory setDefaultType:AnimationTypeLottie];
  [self newAnimatedViewWithType:AnimationTypeLottie
                  animationName:animationName
                     bundleName:bundleName];

  // Title
  self.titleLabel.text =
      [NSString stringWithFormat:@"Left:AnimaX Right:Lottie Name:%@", animationName];
}

- (void)newAnimatedViewWithType:(AnimationType)type
                  animationName:(NSString *)animationName
                     bundleName:(NSString *)bundleName {
  BOOL isAnimaX = (type == AnimationTypeAnimaX);

  LottieAdapterView *adapterView = [[LottieAdapterView alloc] init];

  NSBundle *bundle = nil;
  if (bundleName != nil) {
    bundle = [NSBundle bundleWithURL:[[NSBundle mainBundle] URLForResource:bundleName
                                                             withExtension:@"bundle"]];
  }
  adapterView.layer.masksToBounds = YES;
  [adapterView setContentMode:UIViewContentModeScaleAspectFill];
  if (bundle) {
    [adapterView setAnimationNamed:animationName inBundle:bundle];
  } else {
    [adapterView setAnimationNamed:animationName];
  }
  [adapterView play];
  adapterView.loopAnimation = YES;
  adapterView.userInteractionEnabled = NO;
  adapterView.repeatCount = 5;
  adapterView.autoReverseAnimation = false;

  // Compute view size and position
  CGFloat width =
      floor(CGRectGetWidth(self.view.frame) / 2.0);  // Use floor to ensure the width is an integer.
  CGFloat height = CGRectGetHeight(self.view.frame) - 150;
  CGFloat xPosition = isAnimaX ? 0 : width;

  // Set an accurate frame.
  adapterView.frame = CGRectMake(xPosition, 0, width, height);
  adapterView.contentMode = UIViewContentModeScaleAspectFit;  // Ensure proper content scaling.

  if (isAnimaX) {
    [self removeAdapterView:self.animaXView];
    self.animaXView = adapterView;
  } else {
    [self removeAdapterView:self.lottieView];
    self.lottieView = adapterView;
  }
  [self.view addSubview:adapterView];

  [adapterView play];
}

- (void)setupButtons {
  // Create the main container stackView (vertical direction).
  UIStackView *containerStack = [[UIStackView alloc] init];
  containerStack.axis = UILayoutConstraintAxisVertical;
  containerStack.spacing = 10;
  containerStack.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:containerStack];
  self.buttonStackView = containerStack;

  // Create two rows of buttons (each row is in the horizontal direction).
  UIStackView *topRow = [self createButtonRow:@[ @"Hide All", @"Hide AnimaX" ]];
  UIStackView *middleRow =
      [self createButtonRow:@[ @"Hide Lottie", @"Change Size", @"Open New Page" ]];
  UIStackView *bottomRow = [self createButtonRow:@[ @"Property Update", @"Property Callback" ]];
  UIStackView *lottieAdapterRow = [self createButtonRow:@[ @"LottieAdapter" ]];
  [lottieAdapterRow.arrangedSubviews[0] addTarget:self
                                           action:@selector(onTapLottieAdapterButton:)
                                 forControlEvents:UIControlEventTouchUpInside];

  [containerStack addArrangedSubview:topRow];
  [containerStack addArrangedSubview:middleRow];
  [containerStack addArrangedSubview:bottomRow];
  [containerStack addArrangedSubview:lottieAdapterRow];

  // Set constraints.
  [NSLayoutConstraint activateConstraints:@[
    [containerStack.leftAnchor constraintEqualToAnchor:self.view.leftAnchor constant:20],
    [containerStack.rightAnchor constraintEqualToAnchor:self.view.rightAnchor constant:-20],
    [containerStack.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor constant:-40],
  ]];
}

// Add a helper method to create a row of buttons.
- (UIStackView *)createButtonRow:(NSArray<NSString *> *)buttonTitles {
  UIStackView *rowStack = [[UIStackView alloc] init];
  rowStack.axis = UILayoutConstraintAxisHorizontal;
  rowStack.distribution = UIStackViewDistributionFillEqually;
  rowStack.spacing = 10;
  rowStack.translatesAutoresizingMaskIntoConstraints = NO;
  [rowStack.heightAnchor constraintEqualToConstant:44].active = YES;

  for (NSString *title in buttonTitles) {
    UIButton *button = [UIButton buttonWithType:UIButtonTypeSystem];
    [button setTitle:title forState:UIControlStateNormal];
    button.backgroundColor = [UIColor systemBlueColor];
    [button setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
    button.layer.cornerRadius = 5;
    [rowStack addArrangedSubview:button];

    if ([title isEqualToString:@"Hide All"]) {
      [button addTarget:self
                    action:@selector(toggleVisibility)
          forControlEvents:UIControlEventTouchUpInside];
    } else if ([title isEqualToString:@"Hide AnimaX"]) {
      [button addTarget:self
                    action:@selector(toggleAnimaXVisibility)
          forControlEvents:UIControlEventTouchUpInside];
    } else if ([title isEqualToString:@"Hide Lottie"]) {
      [button addTarget:self
                    action:@selector(toggleLottieVisibility)
          forControlEvents:UIControlEventTouchUpInside];
    } else if ([title isEqualToString:@"Change Size"]) {
      [button addTarget:self
                    action:@selector(changeSize)
          forControlEvents:UIControlEventTouchUpInside];
    } else if ([title isEqualToString:@"Property Update"]) {
      [button addTarget:self
                    action:@selector(openPropertyUpdatePage)
          forControlEvents:UIControlEventTouchUpInside];
    } else if ([title isEqualToString:@"Property Callback"]) {
      [button addTarget:self
                    action:@selector(openPropertyCallbackPage)
          forControlEvents:UIControlEventTouchUpInside];
    } else if (![title isEqualToString:@"LottieAdapter"]) {
      [button addTarget:self
                    action:@selector(openNewPage)
          forControlEvents:UIControlEventTouchUpInside];
    }
  }

  return rowStack;
}

- (void)toggleVisibility {
  self.animaXView.hidden = !self.animaXView.hidden;
  self.lottieView.hidden = !self.lottieView.hidden;
}

- (void)changeSize {
  static BOOL isSmall = NO;
  CGFloat width = CGRectGetWidth(self.view.frame) / 2.0;
  CGFloat height = CGRectGetHeight(self.view.frame) - 150;

  // Modify the frame of the AnimaX view.
  CGRect animaXFrame = self.animaXView.frame;
  animaXFrame.size.width = isSmall ? width : 0;
  animaXFrame.size.height = isSmall ? height : 0;
  self.animaXView.frame = animaXFrame;

  // Modify the frame of the Lottie view.
  CGRect lottieFrame = self.lottieView.frame;
  lottieFrame.size.width = isSmall ? width : 0;
  lottieFrame.size.height = isSmall ? height : 0;
  self.lottieView.frame = lottieFrame;

  isSmall = !isSmall;
}

- (void)openNewPage {
  EmptyViewController *emptyVC = [[EmptyViewController alloc] init];
  emptyVC.modalPresentationStyle = UIModalPresentationFullScreen;
  [self presentViewController:emptyVC animated:YES completion:nil];
}

- (void)addTitleLabel {
  UILabel *label =
      [[UILabel alloc] initWithFrame:CGRectMake(0, CGRectGetHeight(self.view.frame) - 20,
                                                CGRectGetWidth(self.view.frame), 20)];
  label.textAlignment = NSTextAlignmentCenter;
  label.textColor = [UIColor whiteColor];
  label.frame = CGRectMake(0, 100, CGRectGetWidth(self.view.frame), 40);
  [self.view addSubview:label];
  self.titleLabel = label;
}

- (void)removeAdapterView:(LottieAdapterView *)view {
  if (view) {
    [view stop];
    [view removeFromSuperview];
  }
}

- (void)onTapLottieAdapterButton:(UIButton *)button {
  [self.navigationController pushViewController:[[LottieAdapterViewController alloc] init]
                                       animated:YES];
}

- (void)toggleAnimaXVisibility {
  self.animaXView.hidden = !self.animaXView.hidden;
}

- (void)toggleLottieVisibility {
  self.lottieView.hidden = !self.lottieView.hidden;
}

- (void)openPropertyUpdatePage {
  PropertyUpdateController *propertyUpdateVC = [[PropertyUpdateController alloc] init];
  UINavigationController *navController =
      [[UINavigationController alloc] initWithRootViewController:propertyUpdateVC];
  navController.modalPresentationStyle = UIModalPresentationFullScreen;
  [self presentViewController:navController animated:YES completion:nil];
}

- (void)openPropertyCallbackPage {
  PropertyCallbackController *propertyCallbackVC = [[PropertyCallbackController alloc] init];
  UINavigationController *navController =
      [[UINavigationController alloc] initWithRootViewController:propertyCallbackVC];
  navController.modalPresentationStyle = UIModalPresentationFullScreen;
  [self presentViewController:navController animated:YES completion:nil];
}

@end
