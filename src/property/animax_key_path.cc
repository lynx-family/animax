// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "include/property/animax_key_path.h"

#include "base/include/string/string_utils.h"

namespace lynx {
namespace animax {

AnimaXKeyPath::AnimaXKeyPath() : keys_() {}

AnimaXKeyPath::AnimaXKeyPath(const std::vector<std::string>& keys)
    : keys_(keys) {}

AnimaXKeyPath::AnimaXKeyPath(const AnimaXKeyPath& other) : keys_(other.keys_) {}

AnimaXKeyPath AnimaXKeyPath::AddKey(const std::string& key) const {
  std::vector<std::string> new_keys = keys_;
  new_keys.push_back(key);
  return AnimaXKeyPath(new_keys);
}

bool AnimaXKeyPath::Matches(const std::string& key, int32_t depth) const {
  if (IsContainer(key)) {
    // This is an artificial layer we programatically create.
    return true;
  }
  if (depth < 0 || depth >= keys_.size()) {
    return false;
  }
  const std::string& key_at_depth = keys_[depth];
  return key_at_depth == key || key_at_depth == AnimaXKeyPath::kGlobstarKey ||
         key_at_depth == AnimaXKeyPath::kWildcardKey;
}

int32_t AnimaXKeyPath::IncrementDepthBy(const std::string& key,
                                        int32_t depth) const {
  static constexpr int kNoIncrement = 0;
  static constexpr int kIncrementByOne = 1;
  static constexpr int kIncrementByTwo = 2;

  if (IsContainer(key)) {
    // If it's a container then we added programatically and it isn't a part of
    // the keypath.
    return kNoIncrement;
  }

  size_t size = keys_.size();
  // Handle empty keypath or depth out of bounds
  if (depth < 0 || size == 0 || depth >= size) {
    return kIncrementByOne;
  }
  if (keys_[depth] != AnimaXKeyPath::kGlobstarKey) {
    // If it's not a globstar then it is part of the keypath.
    return kIncrementByOne;
  }
  if (depth == size - 1) {
    // The last key is a globstar.
    return kNoIncrement;
  }
  if (keys_[depth + 1] == key) {
    // We are a globstar and the next key is our current key so consume both.
    return kIncrementByTwo;
  }
  return kNoIncrement;
}

bool AnimaXKeyPath::FullyResolvesTo(const std::string& key,
                                    int32_t depth) const {
  auto size = keys_.size();
  if (depth < 0 || depth >= size) {
    return false;
  }
  bool is_last_depth = depth == size - 1;
  const std::string& key_at_depth = keys_[depth];
  bool is_globstar = key_at_depth == AnimaXKeyPath::kGlobstarKey;

  if (!is_globstar) {
    bool matches =
        key_at_depth == key || key_at_depth == AnimaXKeyPath::kWildcardKey;
    return (is_last_depth || (depth == size - 2 && EndsWithGlobstar())) &&
           matches;
  }

  bool is_globstar_but_next_key_matches =
      !is_last_depth && keys_[depth + 1] == key;
  if (is_globstar_but_next_key_matches) {
    return depth == size - 2 || (depth == size - 3 && EndsWithGlobstar());
  }

  if (is_last_depth) {
    return true;
  }
  if (depth + 1 < size - 1) {
    // We are a globstar but there is more than 1 key after the globstar we we
    // can't fully match.
    return false;
  }
  // If the next key (which is the last one) is also a globstar, we can fully
  // resolve
  if (keys_[depth + 1] == AnimaXKeyPath::kGlobstarKey) {
    return true;
  }
  // Return whether the next key (which we now know is the last one) is the same
  // as the current key.
  return keys_[depth + 1] == key;
}

bool AnimaXKeyPath::PropagateToChildren(const std::string& key,
                                        int32_t depth) const {
  if (depth < 0 || key == AnimaXKeyPath::kContainerKey) {
    return true;
  }
  if (keys_.empty()) {
    return false;
  }
  return depth < keys_.size() - 1 ||
         keys_[depth] == AnimaXKeyPath::kGlobstarKey;
}

bool AnimaXKeyPath::IsContainer(const std::string& key) const {
  return key == AnimaXKeyPath::kContainerKey;
}

bool AnimaXKeyPath::EndsWithGlobstar() const {
  return !keys_.empty() &&
         keys_[keys_.size() - 1] == AnimaXKeyPath::kGlobstarKey;
}

std::string AnimaXKeyPath::GetDescription() const {
  return base::Join(keys_, ",");
}

}  // namespace animax
}  // namespace lynx
