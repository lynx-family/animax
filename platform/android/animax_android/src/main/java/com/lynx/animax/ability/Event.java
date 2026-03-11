// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.ability;

public enum Event {
  COMPLETION, // Event when the animation completes playing
  START, // Event when the animation begins to play
  REPEAT, // Event when the animation repeats after completing one cycle
  CANCEL, // Event when the animation playback is canceled
  READY, // Event when the animation is ready to be played (resources are loaded)
  UPDATE, // Event for frame updates during animation playback
  ERROR, // Event when an error occurs during animation processing or playback
  FPS, // Event for reporting frames per second during animation playback
  COMPOSITION_READY, // Event when the composition is ready
  TAP_LAYERS, // Event when the user tap the layers
  FIRST_FRAME, // Event when the first frame is flushed to the surface
  WARNING, // Event when a warning occurs during animation processing or playback
}
