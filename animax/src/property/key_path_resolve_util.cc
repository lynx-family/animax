// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/property/key_path_resolve_util.h"

#include "src/property/key_path_element.h"

namespace lynx {
namespace animax {

ResolutionResult KeyPathResolveUtil::ResolveElement(
    KeyPathElement& element, const AnimaXKeyPath& path, int32_t depth,
    ResolvedKeyPathElements& match_elements,
    const AnimaXKeyPath& current_partial_path) {
  const std::string& element_name = element.GetName();

  bool matches = path.Matches(element_name, depth);
  if (!matches) {
    return {false, depth, current_partial_path};
  }

  // Update partial path for non-container elements
  AnimaXKeyPath updated_partial_path = current_partial_path;
  if (element_name != AnimaXKeyPath::kContainerKey) {
    // Add element to matches if it fully resolves the path
    FullyResolveElement(element, path, depth, match_elements,
                        current_partial_path);
    updated_partial_path = current_partial_path.AddKey(element_name);
  }

  bool should_propagate = path.PropagateToChildren(element_name, depth);
  int32_t child_depth = depth + path.IncrementDepthBy(element_name, depth);

  return {should_propagate, child_depth, std::move(updated_partial_path)};
}

bool KeyPathResolveUtil::FullyResolveElement(
    KeyPathElement& element, const AnimaXKeyPath& path, int32_t depth,
    ResolvedKeyPathElements& match_elements,
    const AnimaXKeyPath& current_partial_path) {
  const std::string& element_name = element.GetName();
  if (path.FullyResolvesTo(element_name, depth)) {
    match_elements.emplace_back(current_partial_path.AddKey(element_name),
                                &element);
    return true;
  }
  return false;
}

}  // namespace animax
}  // namespace lynx
