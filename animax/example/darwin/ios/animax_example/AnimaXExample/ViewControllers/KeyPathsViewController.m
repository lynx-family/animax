// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "KeyPathsViewController.h"
#import <UIKit/UIKit.h>

@interface KeyPathsViewController ()

@property(nonatomic, strong) UITableView *keyPathsTableView;

@end

@implementation KeyPathsViewController

- (instancetype)initWithKeyPaths:(NSArray *)keyPaths {
  self = [super initWithNibName:nil bundle:nil];
  if (self) {
    _keyPaths = [keyPaths copy];
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.view.backgroundColor = [UIColor whiteColor];

  // Add a navigation bar with a close button
  UINavigationBar *navBar = [[UINavigationBar alloc] init];
  navBar.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:navBar];

  UINavigationItem *navItem = [[UINavigationItem alloc] init];
  navItem.title = [NSString stringWithFormat:@"KeyPaths (%lu)", (unsigned long)self.keyPaths.count];

  UIBarButtonItem *closeButton =
      [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemDone
                                                    target:self
                                                    action:@selector(closeButtonTapped)];
  navItem.rightBarButtonItem = closeButton;

  navBar.items = @[ navItem ];

  // Add the table view to display key paths
  self.keyPathsTableView = [[UITableView alloc] init];
  self.keyPathsTableView.translatesAutoresizingMaskIntoConstraints = NO;
  self.keyPathsTableView.dataSource = self;
  self.keyPathsTableView.delegate = self;
  [self.view addSubview:self.keyPathsTableView];

  // Set constraints
  [NSLayoutConstraint activateConstraints:@[
    [navBar.topAnchor constraintEqualToAnchor:self.view.topAnchor],
    [navBar.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
    [navBar.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
    [navBar.heightAnchor constraintEqualToConstant:44],

    [self.keyPathsTableView.topAnchor constraintEqualToAnchor:navBar.bottomAnchor],
    [self.keyPathsTableView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
    [self.keyPathsTableView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
    [self.keyPathsTableView.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor]
  ]];

  // Register cell class
  [self.keyPathsTableView registerClass:[UITableViewCell class]
                 forCellReuseIdentifier:@"KeyPathCell"];
}

- (void)closeButtonTapped {
  [self dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark - UITableViewDataSource

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
  return self.keyPaths.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"KeyPathCell"
                                                          forIndexPath:indexPath];

  NSString *keyPath = self.keyPaths[indexPath.row];
  cell.textLabel.text = [NSString stringWithFormat:@"%ld. %@", (long)(indexPath.row + 1), keyPath];
  cell.textLabel.font = [UIFont systemFontOfSize:14];

  return cell;
}

#pragma mark - UITableViewDelegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  [tableView deselectRowAtIndexPath:indexPath animated:YES];
}

@end
