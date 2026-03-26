// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#ifndef ANIMAX_SRC_RENDER_FONT_H_
#define ANIMAX_SRC_RENDER_FONT_H_

#include <memory>
#include <string>

namespace skity {
class Font;
class FontStyle;
class Typeface;
class TypefaceDelegate;
}  // namespace skity
namespace lynx {
namespace animax {

class Font {
 public:
  static std::shared_ptr<Font> MakeFont(const void* bytes, size_t len,
                                        const std::string& style);
  static std::shared_ptr<Font> MakeFromName(const std::string& family_name,
                                            const std::string& style);

  static std::unique_ptr<skity::TypefaceDelegate> MakeFallbackDelegate();

  static bool HasDefaultTypeface();

  explicit Font(std::unique_ptr<skity::Font>,
                std::unique_ptr<skity::FontStyle>);
  ~Font();

  void SetTextSize(float text_size);

  float MeasureText(const std::string& text) const;

  const skity::Font& GetFont() const;

  const skity::FontStyle& GetFontStyle() const;

  bool HasValidTypeface() const;

 private:
  std::unique_ptr<skity::Font> font_;
  std::unique_ptr<skity::FontStyle> font_style_;
  std::unique_ptr<skity::TypefaceDelegate> delegate_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_RENDER_FONT_H_
