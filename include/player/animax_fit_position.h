// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_INCLUDE_PLAYER_ANIMAX_FIT_POSITION_H_
#define ANIMAX_INCLUDE_PLAYER_ANIMAX_FIT_POSITION_H_

#include <cstdint>

namespace lynx {
namespace animax {
/**
 * Describe how to show the animation when the size of the animation and of
 * the onscreen surface aren't match.
 * Default: kContain
 */
enum class ObjectFit : uint8_t {
  kCenter = 0,  // animation won't be resized, and the animation will be
                // placed on the center of the onscreen surface.
  kCover,    // animation will be proportional scaled. one of the animation side
             // will be scaled to match the onscreen surface, and the other will
             // greater than the onscreen surface. Some parts of animation may
             // be invisible.
  kContain,  // (Default) animation will be proportional scaled. one of the
             // animation side will be scaled to match the onscreen surface,
             // and the other will less than the onscreen surface. It looks
             // like the onscreen surface contains the animation.
  kFill,     // The animation will be scaled to completely fill the
             // onscreen surface, regardless of its aspect ratio. This
             // may cause the animation to be stretched or squished to
             // fit the dimensions of the onscreen surface.
  kScaleDown,  // The animation will be sized as the smaller of:
               // its intrinsic size (no scaling), or
               // a proportionally scaled-down size that fits entirely within
               // the onscreen surface (like "contain" but never upscales).
               // In other words, it only scales down when the intrinsic size
               // would overflow; otherwise it keeps the original size. Aspect
               // ratio is preserved and no part of the animation is clipped.
};

/**
 * Describe how to align the animation within the onscreen surface
 * when the animation size and the surface size do not match.
 * Default: kCenter
 */
enum class ObjectPosition : uint8_t {
  kCenter,  // (Default) The animation will be centered both horizontally
            // and vertically within the onscreen surface.

  kLeft,  // The animation will be aligned to the left edge of the
          // onscreen surface. Vertical alignment remains centered.

  kRight,  // The animation will be aligned to the right edge of the
           // onscreen surface. Vertical alignment remains centered.

  kTop,  // The animation will be aligned to the top edge of the
         // onscreen surface. Horizontal alignment remains centered.

  kBottom,  // The animation will be aligned to the bottom edge of the
            // onscreen surface. Horizontal alignment remains centered.

  kTopLeft,  // The animation will be aligned to the top-left corner of the
             // onscreen surface.

  kTopRight,  // The animation will be aligned to the top-right corner of the
              // onscreen surface.

  kBottomLeft,  // The animation will be aligned to the bottom-left corner of
                // the onscreen surface.

  kBottomRight,  // The animation will be aligned to the bottom-right corner of
                 // the onscreen surface.
};

/**
 * Describe the coordinate space of the layer bounds.
 * Default: kParent
 * Used to get the bounds of the layer for position alignment.
 */
enum class LayerBoundsSpace : uint8_t {
  kParent = 0,  // (Default) Bounds in the parent layer's coordinate space.
  kRoot = 1,    // Bounds in the top-level/root coordinate space.
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_INCLUDE_PLAYER_ANIMAX_FIT_POSITION_H_
