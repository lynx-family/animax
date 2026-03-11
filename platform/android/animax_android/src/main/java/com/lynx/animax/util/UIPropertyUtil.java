// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.util;

import com.lynx.animax.ui.ObjectFit;
import com.lynx.animax.ui.ObjectPosition;

public class UIPropertyUtil {
  public static ObjectFit convertStringToObjectFit(final String objectFitString) {
    ObjectFit objectFitType = ObjectFit.CONTAIN;
    if ("cover".equals(objectFitString)) {
      objectFitType = ObjectFit.COVER;
    } else if ("center".equals(objectFitString)) {
      objectFitType = ObjectFit.CENTER;
    } else if ("fill".equals(objectFitString)) {
      objectFitType = ObjectFit.FILL;
    } else if ("scale-down".equals(objectFitString)) {
      objectFitType = ObjectFit.SCALE_DOWN;
    }

    return objectFitType;
  }

  public static ObjectPosition convertStringToObjectPosition(final String objectPositionString) {
    ObjectPosition objectPositionType = ObjectPosition.CENTER;
    if ("left".equals(objectPositionString)) {
      objectPositionType = ObjectPosition.LEFT;
    } else if ("right".equals(objectPositionString)) {
      objectPositionType = ObjectPosition.RIGHT;
    } else if ("top".equals(objectPositionString)) {
      objectPositionType = ObjectPosition.TOP;
    } else if ("bottom".equals(objectPositionString)) {
      objectPositionType = ObjectPosition.BOTTOM;
    }
    return objectPositionType;
  }
}
