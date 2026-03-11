// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_KEY_PATH_ELEMENT_H_
#define ANIMAX_SRC_PROPERTY_KEY_PATH_ELEMENT_H_

#include <vector>

#include "include/property/animax_key_path.h"
#include "src/property/property_update_element.h"

namespace lynx {
namespace animax {

class KeyPathElement;

/**
 * Type definition for resolved key path elements
 * First element is the resolved path (without wildcards)
 * Second element is the target element
 */
using ResolvedKeyPathElements =
    std::vector<std::pair<AnimaXKeyPath, KeyPathElement*>>;

/**
 * Interface for elements that can be targeted by AnimaX key paths.
 * Elements implementing this interface can be found and updated using key
 * paths.
 */
class KeyPathElement : public PropertyUpdateElement {
 public:
  KeyPathElement() = default;
  virtual ~KeyPathElement() = default;

  /**
   * Resolves a key path and finds all matching elements.
   *
   * @param path The key path to match against
   * @param depth The current depth in the path
   * @param match_elements Output vector to collect resolved path-element pairs
   * @param current_partial_path The current partial resolved path
   */
  virtual void ResolveKeyPath(const AnimaXKeyPath& path, int32_t depth,
                              ResolvedKeyPathElements& match_elements,
                              const AnimaXKeyPath& current_partial_path) = 0;

  /**
   * Returns the name of the element.
   *
   * @return The name of the element
   */
  virtual const std::string& GetName() = 0;
};
}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_KEY_PATH_ELEMENT_H_
