// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/property/animax_key_path.h"

#include "gtest/gtest.h"

using namespace lynx::animax;

TEST(AnimaXKeyPathTest, ConstructorWithVectorKeys) {
  std::vector<std::string> keys = {"layer1", "content", "transform"};
  AnimaXKeyPath keypath(keys);

  EXPECT_EQ(keypath.Size(), 3);
  EXPECT_EQ(keypath.GetKeys().size(), 3);
  EXPECT_EQ(keypath.GetKeys()[0], "layer1");
  EXPECT_EQ(keypath.GetKeys()[1], "content");
  EXPECT_EQ(keypath.GetKeys()[2], "transform");
}

TEST(AnimaXKeyPathTest, ConstructorWithEmptyVector) {
  std::vector<std::string> keys = {};
  AnimaXKeyPath keypath(keys);

  EXPECT_EQ(keypath.Size(), 0);
  EXPECT_TRUE(keypath.GetKeys().empty());
}

TEST(AnimaXKeyPathTest, DefaultConstructor) {
  AnimaXKeyPath keypath;

  EXPECT_EQ(keypath.Size(), 0);
  EXPECT_TRUE(keypath.GetKeys().empty());
}

TEST(AnimaXKeyPathTest, MatchesExactKey) {
  std::vector<std::string> keys = {"layer1", "content", "transform"};
  AnimaXKeyPath keypath(keys);

  EXPECT_TRUE(keypath.Matches("layer1", 0));
  EXPECT_TRUE(keypath.Matches("content", 1));
  EXPECT_TRUE(keypath.Matches("transform", 2));

  EXPECT_FALSE(keypath.Matches("layer2", 0));
  EXPECT_FALSE(keypath.Matches("layer1", 1));
  EXPECT_FALSE(keypath.Matches("transform", 0));
}

TEST(AnimaXKeyPathTest, MatchesWildcard) {
  std::vector<std::string> keys = {AnimaXKeyPath::kWildcardKey, "content",
                                   AnimaXKeyPath::kWildcardKey};
  AnimaXKeyPath keypath(keys);

  EXPECT_TRUE(keypath.Matches("layer1", 0));
  EXPECT_TRUE(keypath.Matches("any_layer", 0));
  EXPECT_TRUE(keypath.Matches("content", 1));
  EXPECT_TRUE(keypath.Matches("any_transform", 2));

  EXPECT_FALSE(keypath.Matches("wrong", 1));
}

TEST(AnimaXKeyPathTest, MatchesGlobstar) {
  std::vector<std::string> keys = {AnimaXKeyPath::kGlobstarKey, "content"};
  AnimaXKeyPath keypath(keys);

  EXPECT_TRUE(keypath.Matches("any_key", 0));
  EXPECT_TRUE(keypath.Matches("content", 1));
  EXPECT_TRUE(keypath.Matches("layer1", 0));
}

TEST(AnimaXKeyPathTest, MatchesContainer) {
  std::vector<std::string> keys = {"layer1", "content"};
  AnimaXKeyPath keypath(keys);

  EXPECT_TRUE(keypath.Matches(AnimaXKeyPath::kContainerKey, 0));
  EXPECT_TRUE(keypath.Matches(AnimaXKeyPath::kContainerKey, 1));
  EXPECT_TRUE(keypath.Matches(AnimaXKeyPath::kContainerKey, 5));
}

TEST(AnimaXKeyPathTest, MatchesDepthOutOfBounds) {
  std::vector<std::string> keys = {"layer1", "content"};
  AnimaXKeyPath keypath(keys);

  EXPECT_FALSE(keypath.Matches("any_key", -1));

  EXPECT_FALSE(keypath.Matches("any_key", 2));
  EXPECT_FALSE(keypath.Matches("any_key", 10));
}

TEST(AnimaXKeyPathTest, IncrementDepthByNormalKey) {
  std::vector<std::string> keys = {"layer1", "content", "transform"};
  AnimaXKeyPath keypath(keys);

  EXPECT_EQ(keypath.IncrementDepthBy("layer1", -1), 1);

  EXPECT_EQ(keypath.IncrementDepthBy("layer1", 0), 1);
  EXPECT_EQ(keypath.IncrementDepthBy("content", 1), 1);
  EXPECT_EQ(keypath.IncrementDepthBy("transform", 2), 1);
}

TEST(AnimaXKeyPathTest, IncrementDepthByContainer) {
  std::vector<std::string> keys = {"layer1", "content"};
  AnimaXKeyPath keypath(keys);

  EXPECT_EQ(keypath.IncrementDepthBy(AnimaXKeyPath::kContainerKey, 0), 0);
  EXPECT_EQ(keypath.IncrementDepthBy(AnimaXKeyPath::kContainerKey, 1), 0);
}

TEST(AnimaXKeyPathTest, IncrementDepthByGlobstarAtEnd) {
  std::vector<std::string> keys = {"layer1", AnimaXKeyPath::kGlobstarKey};
  AnimaXKeyPath keypath(keys);

  EXPECT_EQ(keypath.IncrementDepthBy("layer1", 0), 1);
  EXPECT_EQ(keypath.IncrementDepthBy("any_key", 1), 0);
}

TEST(AnimaXKeyPathTest, IncrementDepthByGlobstarWithMatchingNext) {
  std::vector<std::string> keys = {AnimaXKeyPath::kGlobstarKey, "content",
                                   "transform"};
  AnimaXKeyPath keypath(keys);

  EXPECT_EQ(keypath.IncrementDepthBy("content", 0), 2);
  EXPECT_EQ(keypath.IncrementDepthBy("other", 0), 0);
}

TEST(AnimaXKeyPathTest, IncrementDepthByGlobstarMiddle) {
  std::vector<std::string> keys = {"layer1", AnimaXKeyPath::kGlobstarKey,
                                   "transform"};
  AnimaXKeyPath keypath(keys);

  EXPECT_EQ(keypath.IncrementDepthBy("layer1", 0), 1);
  EXPECT_EQ(keypath.IncrementDepthBy("transform", 1), 2);
  EXPECT_EQ(keypath.IncrementDepthBy("other", 1), 0);
}

TEST(AnimaXKeyPathTest, FullyResolvesToExactMatch) {
  std::vector<std::string> keys = {"layer1", "content", "transform"};
  AnimaXKeyPath keypath(keys);

  EXPECT_FALSE(keypath.FullyResolvesTo("layer1", -1));

  EXPECT_FALSE(keypath.FullyResolvesTo("layer1", 0));
  EXPECT_FALSE(keypath.FullyResolvesTo("content", 1));
  EXPECT_TRUE(keypath.FullyResolvesTo("transform", 2));

  EXPECT_FALSE(keypath.FullyResolvesTo("wrong", 2));
}

TEST(AnimaXKeyPathTest, FullyResolvesToWildcard) {
  std::vector<std::string> keys = {"layer1", "content",
                                   AnimaXKeyPath::kWildcardKey};
  AnimaXKeyPath keypath(keys);

  EXPECT_TRUE(keypath.FullyResolvesTo("any_transform", 2));
  EXPECT_FALSE(keypath.FullyResolvesTo("any_key", 1));
}

TEST(AnimaXKeyPathTest, FullyResolvesToWithGlobstarAtEnd) {
  std::vector<std::string> keys = {"layer1", AnimaXKeyPath::kGlobstarKey};
  AnimaXKeyPath keypath(keys);

  EXPECT_TRUE(keypath.FullyResolvesTo("any_key", 1));
}

TEST(AnimaXKeyPathTest, FullyResolvesToWithGlobstarAndNext) {
  std::vector<std::string> keys = {AnimaXKeyPath::kGlobstarKey, "content"};
  AnimaXKeyPath keypath(keys);

  EXPECT_TRUE(keypath.FullyResolvesTo("content", 0));
  EXPECT_FALSE(keypath.FullyResolvesTo("other", 0));
}

TEST(AnimaXKeyPathTest, FullyResolvesToWithGlobstarAndEndGlobstar) {
  std::vector<std::string> keys = {"layer1", AnimaXKeyPath::kGlobstarKey,
                                   AnimaXKeyPath::kGlobstarKey};
  AnimaXKeyPath keypath(keys);

  EXPECT_FALSE(keypath.FullyResolvesTo("layer1", 0));
  EXPECT_TRUE(keypath.FullyResolvesTo("any_key", 1));
}

TEST(AnimaXKeyPathTest, PropagateToChildren) {
  std::vector<std::string> keys = {"layer1", "content", "transform"};
  AnimaXKeyPath keypath(keys);

  EXPECT_TRUE(keypath.PropagateToChildren("layer1", -1));

  EXPECT_TRUE(keypath.PropagateToChildren("layer1", 0));
  EXPECT_TRUE(keypath.PropagateToChildren("content", 1));
  EXPECT_FALSE(keypath.PropagateToChildren("transform", 2));

  EXPECT_TRUE(keypath.PropagateToChildren(AnimaXKeyPath::kContainerKey, 0));
  EXPECT_TRUE(keypath.PropagateToChildren(AnimaXKeyPath::kContainerKey, 2));
}

TEST(AnimaXKeyPathTest, PropagateToChildrenWithGlobstar) {
  std::vector<std::string> keys = {"layer1", AnimaXKeyPath::kGlobstarKey};
  AnimaXKeyPath keypath(keys);

  EXPECT_TRUE(keypath.PropagateToChildren("layer1", 0));
  EXPECT_TRUE(keypath.PropagateToChildren("any_key", 1));
}

TEST(AnimaXKeyPathTest, EmptyKeyPath) {
  std::vector<std::string> empty_keys;
  AnimaXKeyPath keypath(empty_keys);

  EXPECT_FALSE(keypath.Matches("any_key", 0));
  EXPECT_FALSE(keypath.FullyResolvesTo("any_key", 0));
  EXPECT_FALSE(keypath.PropagateToChildren("any_key", 0));

  EXPECT_TRUE(keypath.Matches(AnimaXKeyPath::kContainerKey, 0));
  EXPECT_TRUE(keypath.PropagateToChildren(AnimaXKeyPath::kContainerKey, 0));
}

TEST(AnimaXKeyPathTest, ComplexGlobstarScenarios) {
  std::vector<std::string> keys = {AnimaXKeyPath::kGlobstarKey, "content",
                                   AnimaXKeyPath::kGlobstarKey};
  AnimaXKeyPath keypath(keys);

  EXPECT_TRUE(keypath.Matches("any1", 0));
  EXPECT_TRUE(keypath.Matches("content", 1));
  EXPECT_TRUE(keypath.Matches("any2", 2));

  EXPECT_EQ(keypath.IncrementDepthBy("content", 0), 2);
  EXPECT_EQ(keypath.IncrementDepthBy("other", 0), 0);
  EXPECT_EQ(keypath.IncrementDepthBy("any", 2), 0);

  EXPECT_TRUE(keypath.FullyResolvesTo("content", 0));
  EXPECT_TRUE(keypath.FullyResolvesTo("any", 2));
}

TEST(AnimaXKeyPathTest, EdgeCasesForIncrementDepthBy) {
  {
    std::vector<std::string> keys = {"layer1"};
    AnimaXKeyPath keypath(keys);

    EXPECT_EQ(keypath.IncrementDepthBy("layer1", 0), 1);
  }

  {
    std::vector<std::string> keys = {AnimaXKeyPath::kGlobstarKey, "target",
                                     "end"};
    AnimaXKeyPath keypath(keys);

    EXPECT_EQ(keypath.IncrementDepthBy("target", 0), 2);
    EXPECT_EQ(keypath.IncrementDepthBy("nomatch", 0), 0);
    EXPECT_EQ(keypath.IncrementDepthBy("end", 1), 1);
  }
}

TEST(AnimaXKeyPathTest, EndsWithGlobstarFunction) {
  {
    std::vector<std::string> keys = {"layer1", AnimaXKeyPath::kGlobstarKey};
    AnimaXKeyPath keypath(keys);

    EXPECT_TRUE(keypath.FullyResolvesTo("layer1", 0));
  }

  {
    std::vector<std::string> keys = {"layer1", "content"};
    AnimaXKeyPath keypath(keys);

    EXPECT_FALSE(keypath.FullyResolvesTo("layer1", 0));
  }
}

TEST(AnimaXKeyPathTest, IsContainerFunction) {
  std::vector<std::string> keys = {"layer1", "content"};
  AnimaXKeyPath keypath(keys);

  EXPECT_TRUE(keypath.Matches(AnimaXKeyPath::kContainerKey, 0));
  EXPECT_EQ(keypath.IncrementDepthBy(AnimaXKeyPath::kContainerKey, 0), 0);
  EXPECT_TRUE(keypath.PropagateToChildren(AnimaXKeyPath::kContainerKey, 0));
  EXPECT_TRUE(keypath.PropagateToChildren(AnimaXKeyPath::kContainerKey, 10));
}

TEST(AnimaXKeyPathTest, SingleGlobstarKeyPath) {
  std::vector<std::string> keys = {AnimaXKeyPath::kGlobstarKey};
  AnimaXKeyPath keypath(keys);

  EXPECT_EQ(keypath.Size(), 1);
  EXPECT_TRUE(keypath.Matches("any_key", 0));
  EXPECT_EQ(keypath.IncrementDepthBy("any_key", 0), 0);
  EXPECT_TRUE(keypath.FullyResolvesTo("any_key", 0));
  EXPECT_TRUE(keypath.PropagateToChildren("any_key", 0));
}

TEST(AnimaXKeyPathTest, SingleWildcardKeyPath) {
  std::vector<std::string> keys = {AnimaXKeyPath::kWildcardKey};
  AnimaXKeyPath keypath(keys);

  EXPECT_EQ(keypath.Size(), 1);
  EXPECT_TRUE(keypath.Matches("any_key", 0));
  EXPECT_EQ(keypath.IncrementDepthBy("any_key", 0), 1);
  EXPECT_TRUE(keypath.FullyResolvesTo("any_key", 0));
  EXPECT_FALSE(keypath.PropagateToChildren("any_key", 0));
}

TEST(AnimaXKeyPathTest, CopyConstructor) {
  std::vector<std::string> keys = {"layer1", "content", "transform"};
  AnimaXKeyPath original(keys);
  AnimaXKeyPath copied(original);

  EXPECT_EQ(copied.Size(), 3);
  EXPECT_EQ(copied.GetKeys().size(), 3);
  EXPECT_EQ(copied.GetKeys()[0], "layer1");
  EXPECT_EQ(copied.GetKeys()[1], "content");
  EXPECT_EQ(copied.GetKeys()[2], "transform");

  // Original should be unchanged
  EXPECT_EQ(original.Size(), 3);
  EXPECT_EQ(original.GetKeys()[0], "layer1");
}

TEST(AnimaXKeyPathTest, AddKeySingleKey) {
  std::vector<std::string> keys = {"layer1"};
  AnimaXKeyPath path(keys);

  AnimaXKeyPath new_path = path.AddKey("content");

  EXPECT_EQ(new_path.Size(), 2);
  EXPECT_EQ(new_path.GetKeys().size(), 2);
  EXPECT_EQ(new_path.GetKeys()[0], "layer1");
  EXPECT_EQ(new_path.GetKeys()[1], "content");

  // Original should be unchanged
  EXPECT_EQ(path.Size(), 1);
  EXPECT_EQ(path.GetKeys()[0], "layer1");
}

TEST(AnimaXKeyPathTest, AddKeyToEmptyPath) {
  std::vector<std::string> empty_keys;
  AnimaXKeyPath path(empty_keys);

  AnimaXKeyPath new_path = path.AddKey("layer1");

  EXPECT_EQ(new_path.Size(), 1);
  EXPECT_EQ(new_path.GetKeys().size(), 1);
  EXPECT_EQ(new_path.GetKeys()[0], "layer1");

  // Original should still be empty
  EXPECT_EQ(path.Size(), 0);
  EXPECT_TRUE(path.GetKeys().empty());
}

TEST(AnimaXKeyPathTest, AddKeyChaining) {
  std::vector<std::string> keys = {"layer1"};
  AnimaXKeyPath path(keys);

  AnimaXKeyPath new_path = path.AddKey("content").AddKey("transform");

  EXPECT_EQ(new_path.Size(), 3);
  EXPECT_EQ(new_path.GetKeys().size(), 3);
  EXPECT_EQ(new_path.GetKeys()[0], "layer1");
  EXPECT_EQ(new_path.GetKeys()[1], "content");
  EXPECT_EQ(new_path.GetKeys()[2], "transform");

  // Original should be unchanged
  EXPECT_EQ(path.Size(), 1);
  EXPECT_EQ(path.GetKeys()[0], "layer1");
}
