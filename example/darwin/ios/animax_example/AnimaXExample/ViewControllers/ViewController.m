// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "ViewController.h"
#import <AnimaX/AnimaXContext.h>
#import <AnimaX/AnimaXView.h>
#import <AnimaX/BaseAnimaXAbility.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "EmptyViewController.h"
#import "PropertyCallbackController.h"
#import "PropertyUpdateController.h"

@interface ViewController ()
@property(nonatomic, strong) UILabel *titleLabel;
@property(nonatomic, strong) AnimaXView *animaXView;
@property(nonatomic, strong) UIStackView *buttonStackView;
@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.view.backgroundColor = [UIColor whiteColor];

  [self addTitleLabel];
  [self setupButtons];
  [self setupAnimation];
}

- (AnimaXView *)createAnimaXView {
  // Create BaseAnimaXAbility and add self as animation listener if needed
  BaseAnimaXAbility *ability = [[BaseAnimaXAbility alloc] init];

  // Create AnimaXContext with the ability
  AnimaXContext *animaxContext = [[AnimaXContext alloc] initWithAbility:ability];

  // Create AnimaXView with the context
  AnimaXView *view = [[AnimaXView alloc] initWithContext:animaxContext];

  // Configure the view settings
  view.enableNativeTapLayerEvent = NO;

  return view;
}

- (void)setupAnimation {
  // Create and configure AnimaXView
  self.animaXView = [self createAnimaXView];
  self.animaXView.translatesAutoresizingMaskIntoConstraints = NO;

  // Set animation properties
  [self.animaXView setAutoplay:YES];
  [self.animaXView setLoop:YES];

  // Load animation
  NSBundle *bundle = [NSBundle mainBundle];
  NSString *jsonPath = [bundle pathForResource:@"animation_1" ofType:@"json"];
  if (jsonPath) {
    NSURL *url = [NSURL fileURLWithPath:jsonPath];
    [self.animaXView setSrc:url.absoluteString];
  }

  // Add to view and set constraints
  [self.view addSubview:self.animaXView];

  [NSLayoutConstraint activateConstraints:@[
    [self.animaXView.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor
                                              constant:16],
    [self.animaXView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:16],
    [self.animaXView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-16],
    [self.animaXView.heightAnchor constraintEqualToConstant:300]
  ]];

  // Update title
  self.titleLabel.text = @"AnimaX Animation Demo";
}

- (void)setupButtons {
  // Create the main container stackView (vertical direction).
  UIStackView *containerStack = [[UIStackView alloc] init];
  containerStack.axis = UILayoutConstraintAxisVertical;
  containerStack.spacing = 10;
  containerStack.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:containerStack];
  self.buttonStackView = containerStack;

  // Create simplified button rows
  UIStackView *topRow = [self createButtonRow:@[ @"Show/Hide", @"Play/Pause" ]];
  UIStackView *middleRow = [self createButtonRow:@[ @"Change Animation", @"Open New Page" ]];
  UIStackView *bottomRow = [self createButtonRow:@[ @"Property Update", @"Property Callback" ]];

  [containerStack addArrangedSubview:topRow];
  [containerStack addArrangedSubview:middleRow];
  [containerStack addArrangedSubview:bottomRow];

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

    if ([title isEqualToString:@"Show/Hide"]) {
      [button addTarget:self
                    action:@selector(toggleVisibility)
          forControlEvents:UIControlEventTouchUpInside];
    } else if ([title isEqualToString:@"Play/Pause"]) {
      [button addTarget:self
                    action:@selector(togglePlayPause)
          forControlEvents:UIControlEventTouchUpInside];
    } else if ([title isEqualToString:@"Change Animation"]) {
      [button addTarget:self
                    action:@selector(changeAnimation)
          forControlEvents:UIControlEventTouchUpInside];
    } else if ([title isEqualToString:@"Property Update"]) {
      [button addTarget:self
                    action:@selector(openPropertyUpdatePage)
          forControlEvents:UIControlEventTouchUpInside];
    } else if ([title isEqualToString:@"Property Callback"]) {
      [button addTarget:self
                    action:@selector(openPropertyCallbackPage)
          forControlEvents:UIControlEventTouchUpInside];
    } else if ([title isEqualToString:@"Open New Page"]) {
      [button addTarget:self
                    action:@selector(openNewPage)
          forControlEvents:UIControlEventTouchUpInside];
    }
  }

  return rowStack;
}

- (void)toggleVisibility {
  self.animaXView.hidden = !self.animaXView.hidden;
}

- (void)togglePlayPause {
  if (self.animaXView.isAnimating) {
    [self.animaXView pause];
  } else {
    [self.animaXView play];
  }
}

- (void)changeAnimation {
  static int animationIndex = 1;
  animationIndex = (animationIndex % 3) + 1;  // Cycle through 1, 2, 3

  NSString *animationName = [NSString stringWithFormat:@"animation_%d", animationIndex];
  NSBundle *bundle = [NSBundle mainBundle];
  NSString *jsonPath = [bundle pathForResource:animationName ofType:@"json"];

  if (jsonPath) {
    NSURL *url = [NSURL fileURLWithPath:jsonPath];
    [self.animaXView setSrc:url.absoluteString];
    self.titleLabel.text = [NSString stringWithFormat:@"AnimaX Animation Demo - %@", animationName];
  }
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
