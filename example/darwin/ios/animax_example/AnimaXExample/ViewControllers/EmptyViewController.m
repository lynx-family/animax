// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#import "EmptyViewController.h"

@implementation EmptyViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  self.view.backgroundColor = [UIColor whiteColor];

  UIButton *backButton = [UIButton buttonWithType:UIButtonTypeSystem];
  [backButton setTitle:@"Back" forState:UIControlStateNormal];
  [backButton addTarget:self
                 action:@selector(backButtonTapped)
       forControlEvents:UIControlEventTouchUpInside];
  backButton.frame = CGRectMake(20, 100, 100, 50);
  [self.view addSubview:backButton];
}

- (void)backButtonTapped {
  [self dismissViewControllerAnimated:YES completion:nil];
}

@end
