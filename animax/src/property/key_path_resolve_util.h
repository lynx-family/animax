// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_PROPERTY_KEY_PATH_RESOLVE_UTIL_H_
#define ANIMAX_SRC_PROPERTY_KEY_PATH_RESOLVE_UTIL_H_

#include "include/property/animax_key_path.h"
#include "src/property/key_path_element.h"

namespace lynx {
namespace animax {

/**
 * Result of key path resolution for a single element.
 */
struct ResolutionResult {
  // Whether to propagate resolution to child elements
  bool should_propagate_to_child_;
  // Depth to use for child resolution
  int32_t child_depth_;
  // Updated key path after resolving the element
  AnimaXKeyPath updated_key_path_;
};

/**
 * Utility class for resolving AnimaX key paths and finding matching elements.
 * Key paths can contain wildcards ('*') and globstars ('**') for flexible
 * targeting.
 */
class KeyPathResolveUtil {
 public:
  /**
   * Helper method to handle common key path resolution logic.
   * This method implements the standard pattern for resolving key paths in
   * elements.
   *
   * @param element The element being resolved
   * @param path The key path to match against
   * @param depth The current depth in the key path
   * @param match_elements Output vector to collect resolved path-element pairs
   * @param current_partial_path The current partial resolved path
   * @param is_matte Whether this is a matte element (skips matches and
   * __container checks)
   * @return Resolution result containing whether to propagate to children and
   * child depth
   */
  static ResolutionResult ResolveElement(
      KeyPathElement& element, const AnimaXKeyPath& path, int32_t depth,
      ResolvedKeyPathElements& match_elements,
      const AnimaXKeyPath& current_partial_path);

  /**
   * Helper method to check if an element fully resolves the key path.
   *
   * @param element The element being resolved
   * @param path The key path to match against
   * @param depth The current depth in the key path
   * @param match_elements Output vector to collect resolved path-element pairs
   * @param current_partial_path The current partial resolved path
   * @return true if the element should be added to matches
   */
  static bool FullyResolveElement(KeyPathElement& element,
                                  const AnimaXKeyPath& path, int32_t depth,
                                  ResolvedKeyPathElements& match_elements,
                                  const AnimaXKeyPath& current_partial_path);
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_PROPERTY_KEY_PATH_RESOLVE_UTIL_H_
