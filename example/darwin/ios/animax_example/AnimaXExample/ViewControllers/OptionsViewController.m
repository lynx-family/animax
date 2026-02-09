// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "OptionsViewController.h"

@interface OptionsViewController ()
@property(nonatomic, strong) UIStackView *containerStack;
@property(nonatomic, assign) BOOL eventsEnabled;
@end

@implementation OptionsViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.view.backgroundColor = [UIColor whiteColor];

  // Add title
  UILabel *titleLabel = [[UILabel alloc] init];
  titleLabel.text = @"Options";
  titleLabel.font = [UIFont boldSystemFontOfSize:18];
  titleLabel.textColor = [UIColor colorWithWhite:0.2 alpha:1.0];
  titleLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:titleLabel];

  // Container Stack
  self.containerStack = [[UIStackView alloc] init];
  self.containerStack.axis = UILayoutConstraintAxisVertical;
  self.containerStack.spacing = 16;
  self.containerStack.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:self.containerStack];

  [NSLayoutConstraint activateConstraints:@[
    [titleLabel.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor
                                         constant:20],
    [titleLabel.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:20],

    [self.containerStack.topAnchor constraintEqualToAnchor:titleLabel.bottomAnchor constant:20],
    [self.containerStack.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:20],
    [self.containerStack.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor
                                                       constant:-20],
    // Don't constrain bottom to allow it to fit content
  ]];

  [self setupOptions];
}

- (void)setupOptions {
  [self addOptionRowWithIcon:@"paintpalette"
                       title:@"Background"
                      action:@selector(backgroundTapped)];
  [self addOptionRowWithIcon:@"list.bullet" title:@"Animations" action:@selector(animationsTapped)];
  [self addOptionRowWithIcon:@"key" title:@"KeyPaths" action:@selector(keyPathsTapped)];
  [self addOptionRowWithIcon:@"hand.tap" title:@"Events" action:@selector(eventsTapped)];
}

- (void)addOptionRowWithIcon:(NSString *)iconName title:(NSString *)title action:(SEL)action {
  UIView *row = [[UIView alloc] init];
  row.translatesAutoresizingMaskIntoConstraints = NO;
  [row.heightAnchor constraintEqualToConstant:44].active = YES;

  // Tap Gesture
  UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:action];
  [row addGestureRecognizer:tap];

  UIImageView *iconView = [[UIImageView alloc] initWithImage:[UIImage systemImageNamed:iconName]];
  iconView.tintColor = [UIColor darkGrayColor];
  iconView.translatesAutoresizingMaskIntoConstraints = NO;
  [row addSubview:iconView];

  UILabel *label = [[UILabel alloc] init];
  label.text = title;
  label.font = [UIFont systemFontOfSize:16];
  label.textColor = [UIColor darkGrayColor];
  label.translatesAutoresizingMaskIntoConstraints = NO;
  [row addSubview:label];

  [NSLayoutConstraint activateConstraints:@[
    [iconView.leadingAnchor constraintEqualToAnchor:row.leadingAnchor],
    [iconView.centerYAnchor constraintEqualToAnchor:row.centerYAnchor],
    [iconView.widthAnchor constraintEqualToConstant:24],
    [iconView.heightAnchor constraintEqualToConstant:24],

    [label.leadingAnchor constraintEqualToAnchor:iconView.trailingAnchor constant:16],
    [label.centerYAnchor constraintEqualToAnchor:row.centerYAnchor],
    [label.trailingAnchor constraintEqualToAnchor:row.trailingAnchor]
  ]];

  [self.containerStack addArrangedSubview:row];
}

#pragma mark - Actions

- (void)backgroundTapped {
  [self.delegate didSelectBackgroundColor];
  [self dismiss];
}

- (void)animationsTapped {
  [self.delegate didSelectAnimationList];
  [self dismiss];
}

- (void)keyPathsTapped {
  [self.delegate didSelectKeyPaths];
  [self dismiss];
}

- (void)eventsTapped {
  self.eventsEnabled = !self.eventsEnabled;
  [self.delegate didToggleEvents:self.eventsEnabled];
  [self dismiss];
}

- (void)dismiss {
  [self dismissViewControllerAnimated:YES completion:nil];
}

@end
