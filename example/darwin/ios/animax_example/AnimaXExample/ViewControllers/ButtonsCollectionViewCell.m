// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "ButtonsCollectionViewCell.h"
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@implementation ButtonsCollectionViewCell

- (instancetype)initWithFrame:(CGRect)frame {
  self = [super initWithFrame:frame];
  if (self) {
    [self setupUI];
  }
  return self;
}

- (void)setupUI {
  self.button = [UIButton buttonWithType:UIButtonTypeSystem];
  self.button.translatesAutoresizingMaskIntoConstraints = NO;
  self.button.backgroundColor = [UIColor systemBlueColor];
  self.button.titleLabel.font = [UIFont systemFontOfSize:14];
  self.button.titleLabel.adjustsFontSizeToFitWidth = YES;
  self.button.titleLabel.minimumScaleFactor = 0.8;
  [self.button setTitleColor:[UIColor whiteColor] forState:UIControlStateNormal];
  self.button.layer.cornerRadius = 5;
  self.button.contentEdgeInsets = UIEdgeInsetsMake(5, 8, 5, 8);

  [self.contentView addSubview:self.button];

  [NSLayoutConstraint activateConstraints:@[
    [self.button.leadingAnchor constraintEqualToAnchor:self.contentView.leadingAnchor],
    [self.button.trailingAnchor constraintEqualToAnchor:self.contentView.trailingAnchor],
    [self.button.topAnchor constraintEqualToAnchor:self.contentView.topAnchor],
    [self.button.bottomAnchor constraintEqualToAnchor:self.contentView.bottomAnchor]
  ]];
}

- (void)setButtonTitle:(NSString *)buttonTitle {
  _buttonTitle = buttonTitle;
  [self.button setTitle:buttonTitle forState:UIControlStateNormal];
}

@end
