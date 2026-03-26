// Copyright 2023 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/render/font.h"

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "skity/text/font_manager.hpp"
#include "skity/text/text_blob.hpp"
#include "skity/text/utf.hpp"
#include "src/base/log/log.h"

namespace lynx {
namespace animax {
namespace {
using StyleMap = std::unordered_map<std::string, skity::FontStyle::Weight>;
using WidthMap = std::unordered_map<std::string, skity::FontStyle::Width>;
using SlantMap = std::unordered_map<std::string, skity::FontStyle::Slant>;

const StyleMap& GetWeightMap() {
  static const StyleMap weight_map = {
      {"thin", skity::FontStyle::kThin_Weight},
      {"extralight", skity::FontStyle::kExtraLight_Weight},
      {"light", skity::FontStyle::kLight_Weight},
      {"medium", skity::FontStyle::kMedium_Weight},
      {"semibold", skity::FontStyle::kSemiBold_Weight},
      {"demibold", skity::FontStyle::kSemiBold_Weight},
      {"bold", skity::FontStyle::kBold_Weight},
      {"extrabold", skity::FontStyle::kExtraBold_Weight},
      {"heavy", skity::FontStyle::kExtraBold_Weight},
      {"black", skity::FontStyle::kBlack_Weight},
      {"extrablack", skity::FontStyle::kExtraBlack_Weight},
      {"ultrablack", skity::FontStyle::kExtraBlack_Weight}};
  return weight_map;
}

const WidthMap& GetWidthMap() {
  static const WidthMap width_map = {
      {"ultracondensed", skity::FontStyle::kUltraCondensed_Width},
      {"extracondensed", skity::FontStyle::kExtraCondensed_Width},
      {"condensed", skity::FontStyle::kCondensed_Width},
      {"semicondensed", skity::FontStyle::kSemiCondensed_Width},
      {"semiexpanded", skity::FontStyle::kSemiExpanded_Width},
      {"expanded", skity::FontStyle::kExpanded_Width},
      {"extraexpanded", skity::FontStyle::kExtraExpanded_Width},
      {"ultraexpanded", skity::FontStyle::kUltraExpanded_Width}};
  return width_map;
}

const SlantMap& GetSlantMap() {
  static const SlantMap slant_map = {
      {"italic", skity::FontStyle::kItalic_Slant},
      {"oblique", skity::FontStyle::kOblique_Slant}};
  return slant_map;
}

skity::FontStyle ParseFontStyle(const std::string& style) {
  if (style.empty()) {
    return skity::FontStyle();
  }

  auto weight = skity::FontStyle::kNormal_Weight;
  auto width = skity::FontStyle::kNormal_Width;
  auto slant = skity::FontStyle::kUpright_Slant;

  std::string token;
  std::istringstream stream(style);

  while (std::getline(stream, token, ' ')) {
    // Remove hyphens and convert to lowercase.
    token.erase(std::remove(token.begin(), token.end(), '-'), token.end());
    std::transform(token.begin(), token.end(), token.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    // Parse weight
    auto weight_it = GetWeightMap().find(token);
    if (weight_it != GetWeightMap().end()) {
      weight = weight_it->second;
      continue;
    }

    // Parse width
    auto width_it = GetWidthMap().find(token);
    if (width_it != GetWidthMap().end()) {
      width = width_it->second;
      continue;
    }

    // Parse slant
    auto slant_it = GetSlantMap().find(token);
    if (slant_it != GetSlantMap().end()) {
      slant = slant_it->second;
      continue;
    }
  }

  return skity::FontStyle(weight, width, slant);
}
}  // namespace

class FallbackTypefaceDelegate : public skity::TypefaceDelegate {
 public:
  FallbackTypefaceDelegate() = default;
  ~FallbackTypefaceDelegate() = default;

  std::shared_ptr<skity::Typeface> Fallback(skity::Unichar code_point,
                                            skity::Paint const& text_paint);

  std::vector<std::vector<skity::Unichar>> BreakTextRun(const char* text) {
    return {};
  }
};

std::unique_ptr<skity::TypefaceDelegate> Font::MakeFallbackDelegate() {
  return std::unique_ptr<skity::TypefaceDelegate>(
      new FallbackTypefaceDelegate());
}

std::shared_ptr<Font> Font::MakeFont(const void* bytes, size_t len,
                                     const std::string& style) {
  skity::FontStyle font_style = ParseFontStyle(style);
  const auto data = skity::Data::MakeWithProc(bytes, len, nullptr, nullptr);
  auto typeface = skity::Typeface::MakeFromData(data);
  typeface =
      typeface ? typeface : skity::Typeface::GetDefaultTypeface(font_style);
  return std::make_shared<Font>(std::make_unique<skity::Font>(typeface),
                                std::make_unique<skity::FontStyle>(font_style));
}

std::shared_ptr<Font> Font::MakeFromName(const std::string& family_name,
                                         const std::string& style) {
  skity::FontStyle font_style = ParseFontStyle(style);
  auto typeface = skity::FontManager::RefDefault()->MatchFamilyStyle(
      family_name.c_str(), font_style);
  if (!typeface) {
    typeface = skity::Typeface::GetDefaultTypeface(font_style);
    if (!typeface) {
      ANIMAX_LOGE("Font::MakeFromName return font for font "
                  << family_name.c_str() << " get default font null");
    }
  }
  return std::make_shared<Font>(std::make_unique<skity::Font>(typeface),
                                std::make_unique<skity::FontStyle>(font_style));
}

bool Font::HasDefaultTypeface() {
  return skity::Typeface::GetDefaultTypeface() != nullptr;
}

Font::Font(std::unique_ptr<skity::Font> font,
           std::unique_ptr<skity::FontStyle> font_style)
    : font_(std::move(font)), font_style_(std::move(font_style)) {
  DCHECK(font_ && font_style_);
  delegate_ =
      std::unique_ptr<FallbackTypefaceDelegate>(new FallbackTypefaceDelegate());
}

Font::~Font() = default;

void Font::SetTextSize(float text_size) { font_->SetSize(text_size); }

const skity::Font& Font::GetFont() const { return *font_; }

const skity::FontStyle& Font::GetFontStyle() const { return *font_style_; }

bool Font::HasValidTypeface() const { return font_->GetTypeface() != nullptr; }

float Font::MeasureText(const std::string& text) const {
  skity::Paint paint;
  paint.SetTextSize(font_->GetSize());
  paint.SetTypeface(font_->GetTypeface());

  skity::TextBlobBuilder builder;
  auto blob = builder.BuildTextBlob(text.c_str(), paint, delegate_.get());

  if (blob) {
    auto bounds = blob->GetBoundSize();
    return bounds.x;
  }

  return 0;
}

std::shared_ptr<skity::Typeface> FallbackTypefaceDelegate::Fallback(
    skity::Unichar code_point, skity::Paint const& text_paint) {
  auto font_manager = skity::FontManager::RefDefault();

  return font_manager->MatchFamilyStyleCharacter(0, skity::FontStyle(), nullptr,
                                                 0, code_point);
}

}  // namespace animax
}  // namespace lynx
