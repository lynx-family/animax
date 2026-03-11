// Copyright 2023 The Lynx Authors. All rights reserved.

#ifndef ANIMAX_SRC_BASE_UTIL_MISC_UTIL_H_
#define ANIMAX_SRC_BASE_UTIL_MISC_UTIL_H_

#include <string>

#include "src/model/value/base_value.h"

namespace lynx {
namespace animax {

/**
 * Performs linear interpolation between two floating-point values.
 * @param a The start value.
 * @param b The end value.
 * @param percentage The weight of 'b' in the result.
 * @return The interpolated result as a floating-point value.
 */
float Lerp(const float& a, const float& b, const float& percentage);

/**
 * Performs linear interpolation between two integer values.
 * @param a The start value.
 * @param b The end value.
 * @param percentage The weight of 'b' in the result.
 * @return The interpolated result as an integer, rounded towards zero.
 */
int32_t Lerp(const int32_t& a, const int32_t& b, const float& percentage);

/**
 * Overloaded Lerp function for custom Integer objects.
 * @param a The start Integer value.
 * @param b The end Integer value.
 * @param percentage The weight of 'b' in the result.
 * @param out Pointer to Integer where the result is stored.
 */
void Lerp(const Integer& a, const Integer& b, const float& percentage,
          Integer* const out);

/**
 * Overloaded Lerp function for custom Float objects.
 * @param a The start Float value.
 * @param b The end Float value.
 * @param percentage The weight of 'b' in the result.
 * @param out Pointer to Float where the result is stored.
 */
void Lerp(const Float& a, const Float& b, const float& percentage,
          Float* const out);

/**
 * Computes the floor modulus of two integers.
 * @param x The dividend.
 * @param y The divisor.
 * @return The remainder of the division that rounds the quotient towards
 * negative infinity.
 */
int32_t FloorMod(const int32_t& x, const int32_t& y);

/**
 * Calculates a color resulting from the interpolation between two colors over
 * the progress value.
 * @param start_color The initial color state.
 * @param end_color The final color state.
 * @param progress The point in the interpolation.
 * @param out Pointer to Color where the result is stored.
 */
void GammaEvaluate(const Color& start_color, const Color& end_color,
                   const float& progress, Color* const out);

/**
 * Overloaded function of GammaEvaluate that operates directly on integer
 * representations of color. This version performs the operation assuming the
 * color values are encoded as integers.
 * @param start_color The initial color state as an integer.
 * @param end_color The final color state as an integer.
 * @param progress The point in the interpolation.
 * @return The interpolated result as color int
 */
int32_t GammaEvaluate(const int32_t& start_color, const int32_t& end_color,
                      const float& progress);

/**
 * Validates a given URL.
 * Checks if the URL starts with "http" or "asset" to determine if it's a valid
 * resource link.
 * @param url The URL to be validated.
 * @return 'true' if the URL starts with "http" or "asset", 'false' otherwise.
 */
bool IsUrlValid(const std::string& url);

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_BASE_UTIL_MISC_UTIL_H_
