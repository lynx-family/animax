// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#import "ButtonsCollectionViewDataSource.h"
#import <UIKit/UIKit.h>
#import "ButtonsCollectionViewCell.h"

@interface ButtonsCollectionViewDataSource ()

@property(nonatomic, strong) NSMutableArray<NSString *> *buttonTitles;

@end

@implementation ButtonsCollectionViewDataSource

- (instancetype)initWithButtonTitles:(NSArray<NSString *> *)buttonTitles {
  self = [super init];
  if (self) {
    _buttonTitles = [buttonTitles mutableCopy];
  }
  return self;
}

- (NSInteger)collectionView:(UICollectionView *)collectionView
     numberOfItemsInSection:(NSInteger)section {
  return self.buttonTitles.count;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView
                  cellForItemAtIndexPath:(NSIndexPath *)indexPath {
  ButtonsCollectionViewCell *cell = (ButtonsCollectionViewCell *)[collectionView
      dequeueReusableCellWithReuseIdentifier:@"ButtonCell"
                                forIndexPath:indexPath];
  NSString *title = self.buttonTitles[indexPath.item];
  cell.buttonTitle = title;
  return cell;
}

- (void)addButtonWithTitle:(NSString *)title {
  [self.buttonTitles addObject:title];
}

@end
