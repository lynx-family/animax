// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/key_path_resolve_util.h"

#include "gtest/gtest.h"
#include "include/property/animax_key_path.h"

using namespace lynx::animax;

// Mock implementation of KeyPathElement for testing
class MockKeyPathElement : public KeyPathElement {
 public:
  explicit MockKeyPathElement(const std::string& name) : name_(name) {}

  void ResolveKeyPath(const AnimaXKeyPath& path, int32_t depth,
                      ResolvedKeyPathElements& match_elements,
                      const AnimaXKeyPath& current_partial_path) override {}

  const std::string& GetName() override { return name_; }

 private:
  std::string name_;
};

TEST(KeyPathResolveUtilTest, ResolveElementExactMatch) {
  MockKeyPathElement element("layer1");
  std::vector<std::string> keys = {"layer1"};
  AnimaXKeyPath path(keys);
  ResolvedKeyPathElements match_elements;

  ResolutionResult result = KeyPathResolveUtil::ResolveElement(
      element, path, 0, match_elements, AnimaXKeyPath());

  EXPECT_FALSE(result.should_propagate_to_child_);
  EXPECT_EQ(result.child_depth_, 1);
  EXPECT_EQ(match_elements.size(), 1);
  EXPECT_EQ(match_elements[0].second, &element);
  EXPECT_EQ(match_elements[0].first.GetKeys().size(), 1);
  EXPECT_EQ(match_elements[0].first.GetKeys()[0], "layer1");
}

TEST(KeyPathResolveUtilTest, ResolveElementNoMatch) {
  MockKeyPathElement element("layer1");
  std::vector<std::string> keys = {"layer2"};
  AnimaXKeyPath path(keys);
  ResolvedKeyPathElements match_elements;

  ResolutionResult result = KeyPathResolveUtil::ResolveElement(
      element, path, 0, match_elements, AnimaXKeyPath());

  EXPECT_FALSE(result.should_propagate_to_child_);
  EXPECT_EQ(result.child_depth_, 0);
  EXPECT_EQ(match_elements.size(), 0);
}

TEST(KeyPathResolveUtilTest, ResolveElementWildcardMatch) {
  MockKeyPathElement element("layer1");
  std::vector<std::string> keys = {"*"};
  AnimaXKeyPath path(keys);
  ResolvedKeyPathElements match_elements;

  ResolutionResult result = KeyPathResolveUtil::ResolveElement(
      element, path, 0, match_elements, AnimaXKeyPath());

  EXPECT_FALSE(result.should_propagate_to_child_);
  EXPECT_EQ(result.child_depth_, 1);
  EXPECT_EQ(match_elements.size(), 1);
  EXPECT_EQ(match_elements[0].second, &element);
  EXPECT_EQ(match_elements[0].first.GetKeys().size(), 1);
  EXPECT_EQ(match_elements[0].first.GetKeys()[0], "layer1");
}

TEST(KeyPathResolveUtilTest, ResolveElementGlobstarMatch) {
  MockKeyPathElement element("layer1");
  std::vector<std::string> keys = {"**"};
  AnimaXKeyPath path(keys);
  ResolvedKeyPathElements match_elements;

  ResolutionResult result = KeyPathResolveUtil::ResolveElement(
      element, path, 0, match_elements, AnimaXKeyPath());

  EXPECT_TRUE(result.should_propagate_to_child_);
  EXPECT_EQ(result.child_depth_, 0);
  EXPECT_EQ(match_elements.size(), 1);
  EXPECT_EQ(match_elements[0].second, &element);
  EXPECT_EQ(match_elements[0].first.GetKeys().size(), 1);
  EXPECT_EQ(match_elements[0].first.GetKeys()[0], "layer1");
}

TEST(KeyPathResolveUtilTest, ResolveElementContainerKey) {
  MockKeyPathElement element(AnimaXKeyPath::kContainerKey);
  std::vector<std::string> keys = {"layer1"};
  AnimaXKeyPath path(keys);
  ResolvedKeyPathElements match_elements;

  ResolutionResult result = KeyPathResolveUtil::ResolveElement(
      element, path, 0, match_elements, AnimaXKeyPath());

  EXPECT_TRUE(result.should_propagate_to_child_);
  EXPECT_EQ(result.child_depth_, 0);
  EXPECT_EQ(match_elements.size(), 0);
}

TEST(KeyPathResolveUtilTest, ResolveElementMatteSkipsMatchesCheck) {
  MockKeyPathElement element("layer1");
  std::vector<std::string> keys = {"layer2"};
  AnimaXKeyPath path(keys);
  ResolvedKeyPathElements match_elements;

  ResolutionResult result = KeyPathResolveUtil::ResolveElement(
      element, path, 0, match_elements, AnimaXKeyPath());

  EXPECT_FALSE(result.should_propagate_to_child_);
  EXPECT_EQ(result.child_depth_, 0);
  EXPECT_EQ(match_elements.size(), 0);
}

TEST(KeyPathResolveUtilTest, ResolveElementMatteContainerStillAdded) {
  MockKeyPathElement element(AnimaXKeyPath::kContainerKey);
  std::vector<std::string> keys = {AnimaXKeyPath::kContainerKey};
  AnimaXKeyPath path(keys);
  ResolvedKeyPathElements match_elements;

  ResolutionResult result = KeyPathResolveUtil::ResolveElement(
      element, path, 0, match_elements, AnimaXKeyPath());

  EXPECT_TRUE(result.should_propagate_to_child_);
  EXPECT_EQ(result.child_depth_, 0);
  EXPECT_EQ(match_elements.size(), 0);
}

TEST(KeyPathResolveUtilTest, ResolveElementMultiLevelPath) {
  MockKeyPathElement element("layer1");
  std::vector<std::string> keys = {"layer1", "content", "transform"};
  AnimaXKeyPath path(keys);
  ResolvedKeyPathElements match_elements;

  ResolutionResult result = KeyPathResolveUtil::ResolveElement(
      element, path, 0, match_elements, AnimaXKeyPath());

  EXPECT_TRUE(result.should_propagate_to_child_);
  EXPECT_EQ(result.child_depth_, 1);
  EXPECT_EQ(match_elements.size(), 0);  // Not fully resolved
}

TEST(KeyPathResolveUtilTest, ResolveElementWithPartialPath) {
  MockKeyPathElement element("content");
  std::vector<std::string> keys = {"layer1", "content"};
  AnimaXKeyPath path(keys);
  std::vector<std::string> partial_keys = {"layer1"};
  AnimaXKeyPath partial_path(partial_keys);
  ResolvedKeyPathElements match_elements;

  ResolutionResult result = KeyPathResolveUtil::ResolveElement(
      element, path, 1, match_elements, partial_path);

  EXPECT_FALSE(result.should_propagate_to_child_);
  EXPECT_EQ(result.child_depth_, 2);
  EXPECT_EQ(match_elements.size(), 1);
  EXPECT_EQ(match_elements[0].second, &element);
  EXPECT_EQ(match_elements[0].first.GetKeys().size(), 2);
  EXPECT_EQ(match_elements[0].first.GetKeys()[0], "layer1");
  EXPECT_EQ(match_elements[0].first.GetKeys()[1], "content");
}

TEST(KeyPathResolveUtilTest, FullyResolveElementExactMatch) {
  MockKeyPathElement element("layer1");
  std::vector<std::string> keys = {"layer1"};
  AnimaXKeyPath path(keys);
  ResolvedKeyPathElements match_elements;

  bool result = KeyPathResolveUtil::FullyResolveElement(
      element, path, 0, match_elements, AnimaXKeyPath());

  EXPECT_TRUE(result);
  EXPECT_EQ(match_elements.size(), 1);
  EXPECT_EQ(match_elements[0].second, &element);
  EXPECT_EQ(match_elements[0].first.GetKeys().size(), 1);
  EXPECT_EQ(match_elements[0].first.GetKeys()[0], "layer1");
}

TEST(KeyPathResolveUtilTest, FullyResolveElementContainer) {
  MockKeyPathElement element(AnimaXKeyPath::kContainerKey);
  std::vector<std::string> keys = {AnimaXKeyPath::kContainerKey};
  AnimaXKeyPath path(keys);
  ResolvedKeyPathElements match_elements;

  bool result = KeyPathResolveUtil::FullyResolveElement(
      element, path, 0, match_elements, AnimaXKeyPath());

  EXPECT_TRUE(result);
  EXPECT_EQ(match_elements.size(), 1);
}

TEST(KeyPathResolveUtilTest, AddKeyMethod) {
  std::vector<std::string> keys = {"layer1"};
  AnimaXKeyPath path(keys);

  AnimaXKeyPath new_path = path.AddKey("content");

  EXPECT_EQ(new_path.GetKeys().size(), 2);
  EXPECT_EQ(new_path.GetKeys()[0], "layer1");
  EXPECT_EQ(new_path.GetKeys()[1], "content");

  // Original path should be unchanged
  EXPECT_EQ(path.GetKeys().size(), 1);
  EXPECT_EQ(path.GetKeys()[0], "layer1");
}
