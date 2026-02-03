// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_MODEL_VALUE_BLEND_MODE_TYPE_H_
#define ANIMAX_SRC_MODEL_VALUE_BLEND_MODE_TYPE_H_

#include <cstdint>

namespace lynx {
namespace animax {

// same with skity::BlendMode
enum class BlendModeType : int32_t {
  kClear,       //!< r = 0
  kSrc,         //!< r = s
  kDst,         //!< r = d
  kSrcOver,     //!< r = s + (1-sa)*d
  kDstOver,     //!< r = d + (1-da)*s
  kSrcIn,       //!< r = s * da
  kDstIn,       //!< r = d * sa
  kSrcOut,      //!< r = s * (1-da)
  kDstOut,      //!< r = d * (1-sa)
  kSrcATop,     //!< r = s*da + d*(1-sa)
  kDstATop,     //!< r = d*sa + s*(1-da)
  kXor,         //!< r = s*(1-da) + d*(1-sa)
  kPlus,        //!< r = min(s + d, 1)
  kModulate,    //!< r = s*d
  kScreen,      //!< r = s + d - s*d
  kOverlay,     //!< multiply or screen, depending on destination
  kDarken,      //!< rc = s + d - max(s*da, d*sa), ra = kSrcOver
  kLighten,     //!< rc = s + d - min(s*da, d*sa), ra = kSrcOver
  kColorDodge,  //!< brighten destination to reflect source
  kColorBurn,   //!< darken destination to reflect source
  kHardLight,   //!< multiply or screen, depending on source
  kSoftLight,   //!< lighten or darken, depending on source
  kDifference,  //!< rc = s + d - 2*(min(s*da, d*sa)), ra = kSrcOver
  kExclusion,   //!< rc = s + d - two(s*d), ra = kSrcOver
  kMultiply,    //!< r = s*(1-da) + d*(1-sa) + s*d
  kHue,         //!< hue of source with saturation and luminosity of destination
  kSaturation,  //!< saturation of source with hue and luminosity of destination
  kColor,       //!< hue and saturation of source with luminosity of destination
  kLuminosity,  //!< luminosity of source with hue and saturation of destination
  kLastCoeffMode = kScreen,  //!< last porter duff blend mode
  kLastSeparableMode =
      kMultiply,  //!< last blend mode operating separately on components
  kDefault = kSrcOver,
  kNormal = kSrcOver,
  kLastMode = kLuminosity,  //!< last valid value
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_MODEL_VALUE_BLEND_MODE_TYPE_H_
