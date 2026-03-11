// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PROPERTY_ANIMAX_KEY_PATH_H_
#define ANIMAX_INCLUDE_PROPERTY_ANIMAX_KEY_PATH_H_

#include <cstdint>
#include <string>
#include <vector>

#include "include/base/macros.h"

namespace lynx {
namespace animax {

/**
 * A path that can target specific layers or elements in an AnimaX animation.
 *
 * KeyPath supports wildcards ('*') to match exactly one item and globstars
 * ('**') to match zero or more items, enabling flexible targeting of animation
 * elements.
 */
class ANIMAX_EXPORT AnimaXKeyPath {
 public:
  static constexpr const char* kContainerKey = "__container";
  static constexpr const char* kGlobstarKey = "**";
  static constexpr const char* kWildcardKey = "*";

  AnimaXKeyPath();

  /**
   * Creates a KeyPath with the specified key segments
   * @param keys The key segments that define the path
   */
  explicit AnimaXKeyPath(const std::vector<std::string>& keys);

  /**
   * Copy constructor
   * @param other The KeyPath to copy from
   */
  AnimaXKeyPath(const AnimaXKeyPath& other);

  virtual ~AnimaXKeyPath() = default;

  /**
   * Checks if the key at the specified depth matches the given key
   * @param key The key to match against
   * @param depth The depth in the path
   * @return true if the key matches at the specified depth
   */
  bool Matches(const std::string& key, int32_t depth) const;

  /**
   * For a given key and depth, returns how much the depth should be incremented
   * by when resolving a keypath to children.
   * @param key The key to check
   * @param depth The current depth
   * @return The depth increment (0, 1, or 2)
   */
  int32_t IncrementDepthBy(const std::string& key, int32_t depth) const;

  /**
   * Returns whether the key at specified depth is fully specific enough to
   * match the full set of keys in this keypath.
   * @param key The key to check
   * @param depth The current depth
   * @return true if the key fully resolves at the current depth
   */
  bool FullyResolvesTo(const std::string& key, int32_t depth) const;

  /**
   * Returns whether the keypath resolution should propagate to children.
   * @param key The key to check
   * @param depth The current depth
   * @return true if resolution should continue to children
   */
  bool PropagateToChildren(const std::string& key, int32_t depth) const;

  /**
   * Gets the number of keys in this KeyPath
   * @return Number of keys
   */
  size_t Size() const { return keys_.size(); }

  /**
   * Gets all keys as a vector
   * @return Vector of all keys
   */
  const std::vector<std::string>& GetKeys() const { return keys_; }

  /**
   * Gets the description of this KeyPath, with keys joined by commas.
   * @return The description string.
   */
  std::string GetDescription() const;

  /**
   * Creates a new KeyPath by adding a key to this one
   * @param key The key to add
   * @return A new KeyPath with the added key
   */
  AnimaXKeyPath AddKey(const std::string& key) const;

 private:
  /**
   * Checks if a key represents a container element
   * @param key The key to check
   * @return true if the key represents a container
   */
  bool IsContainer(const std::string& key) const;

  /**
   * Checks if the keypath ends with a globstar
   * @return true if the last key is "**"
   */
  bool EndsWithGlobstar() const;

  std::vector<std::string> keys_ = {};
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PROPERTY_ANIMAX_KEY_PATH_H_
