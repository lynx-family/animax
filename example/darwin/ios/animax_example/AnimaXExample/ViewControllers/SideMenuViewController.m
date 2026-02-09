// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "SideMenuViewController.h"

@interface SideMenuViewController ()

@property(nonatomic, strong) UIView *dimmingView;
@property(nonatomic, strong) UIView *menuView;
@property(nonatomic, strong) UIStackView *contentStack;

@end

@implementation SideMenuViewController

- (void)viewDidLoad {
  [super viewDidLoad];

  self.view.backgroundColor = [UIColor clearColor];

  // Dimming View
  self.dimmingView = [[UIView alloc] init];
  self.dimmingView.backgroundColor = [UIColor colorWithWhite:0.0 alpha:0.5];
  self.dimmingView.alpha = 0.0;
  self.dimmingView.translatesAutoresizingMaskIntoConstraints = NO;
  UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self
                                                                        action:@selector(dismiss)];
  [self.dimmingView addGestureRecognizer:tap];
  [self.view addSubview:self.dimmingView];

  // Menu View (Drawer)
  self.menuView = [[UIView alloc] init];
  self.menuView.backgroundColor = [UIColor whiteColor];
  self.menuView.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:self.menuView];

  // Content Stack
  self.contentStack = [[UIStackView alloc] init];
  self.contentStack.axis = UILayoutConstraintAxisVertical;
  self.contentStack.spacing = 0;
  self.contentStack.translatesAutoresizingMaskIntoConstraints = NO;
  [self.menuView addSubview:self.contentStack];

  // Header
  // Header Container
  UIView *headerContainer = [[UIView alloc] init];
  [headerContainer.heightAnchor constraintEqualToConstant:100].active = YES;
  [self.contentStack addArrangedSubview:headerContainer];

  UILabel *headerLabel = [[UILabel alloc] init];
  headerLabel.text = @"AnimaX Previewer";
  headerLabel.font = [UIFont boldSystemFontOfSize:20];
  headerLabel.textAlignment = NSTextAlignmentLeft;
  headerLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [headerContainer addSubview:headerLabel];

  [NSLayoutConstraint activateConstraints:@[
    [headerLabel.leadingAnchor constraintEqualToAnchor:headerContainer.leadingAnchor constant:24],
    [headerLabel.centerYAnchor constraintEqualToAnchor:headerContainer.centerYAnchor]
  ]];

  // Separator
  UIView *line = [[UIView alloc] init];
  line.backgroundColor = [UIColor colorWithWhite:0.9 alpha:1.0];
  [line.heightAnchor constraintEqualToConstant:1].active = YES;
  [self.contentStack addArrangedSubview:line];

  // Menu Items
  [self addMenuItemWithTitle:@"Property Update" action:@selector(propertyUpdateTapped)];
  [self addMenuItemWithTitle:@"Property Callback" action:@selector(propertyCallbackTapped)];
  [self addMenuItemWithTitle:@"Multi Animation" action:@selector(multiAnimationTapped)];

  // Constraints
  [NSLayoutConstraint activateConstraints:@[
    [self.dimmingView.topAnchor constraintEqualToAnchor:self.view.topAnchor],
    [self.dimmingView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
    [self.dimmingView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
    [self.dimmingView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor],

    [self.menuView.topAnchor constraintEqualToAnchor:self.view.topAnchor],
    [self.menuView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor],
    [self.menuView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
    [self.menuView.widthAnchor constraintEqualToConstant:280],

    [self.contentStack.topAnchor
        constraintEqualToAnchor:self.menuView.safeAreaLayoutGuide.topAnchor],
    [self.contentStack.leadingAnchor constraintEqualToAnchor:self.menuView.leadingAnchor],
    [self.contentStack.trailingAnchor constraintEqualToAnchor:self.menuView.trailingAnchor],
  ]];

  // Initial State off-screen
  self.menuView.transform = CGAffineTransformMakeTranslation(-280, 0);
}

- (void)addMenuItemWithTitle:(NSString *)title action:(SEL)action {
  UIButton *btn = [UIButton buttonWithType:UIButtonTypeSystem];
  [btn setTitle:title forState:UIControlStateNormal];
  [btn setContentHorizontalAlignment:UIControlContentHorizontalAlignmentLeft];

  // Align text with header label (24pt leading)
  btn.contentEdgeInsets = UIEdgeInsetsMake(0, 24, 0, 0);

  btn.tintColor = [UIColor darkGrayColor];
  [btn addTarget:self action:action forControlEvents:UIControlEventTouchUpInside];
  [btn.heightAnchor constraintEqualToConstant:56].active = YES;

  [self.contentStack addArrangedSubview:btn];
}

#pragma mark - Actions

- (void)propertyUpdateTapped {
  [self dismissWithCompletion:^{
    [self.delegate didSelectPropertyUpdate];
  }];
}

- (void)propertyCallbackTapped {
  [self dismissWithCompletion:^{
    [self.delegate didSelectPropertyCallback];
  }];
}

- (void)multiAnimationTapped {
  [self dismissWithCompletion:^{
    [self.delegate didSelectMultiAnimation];
  }];
}

#pragma mark - Presentation

- (void)showInViewController:(UIViewController *)viewController {
  self.modalPresentationStyle = UIModalPresentationOverFullScreen;
  self.modalTransitionStyle = UIModalTransitionStyleCrossDissolve;
  [viewController presentViewController:self
                               animated:NO
                             completion:^{
                               [UIView animateWithDuration:0.3
                                                     delay:0
                                                   options:UIViewAnimationOptionCurveEaseOut
                                                animations:^{
                                                  self.dimmingView.alpha = 1.0;
                                                  self.menuView.transform =
                                                      CGAffineTransformIdentity;
                                                }
                                                completion:nil];
                             }];
}

- (void)dismiss {
  [self dismissWithCompletion:nil];
}

- (void)dismissWithCompletion:(void (^)(void))completion {
  [UIView animateWithDuration:0.3
      delay:0
      options:UIViewAnimationOptionCurveEaseIn
      animations:^{
        self.dimmingView.alpha = 0.0;
        self.menuView.transform = CGAffineTransformMakeTranslation(-280, 0);
      }
      completion:^(BOOL finished) {
        [self dismissViewControllerAnimated:NO completion:completion];
      }];
}

@end
