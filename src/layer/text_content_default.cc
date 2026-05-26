// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
// Copyright 2018 Airbnb, Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/layer/text_content_default.h"

#include "base/include/string/string_utils.h"
#include "src/base/log/log.h"
#include "src/model/value/document_data.h"
#include "src/resource/asset/font_asset.h"

namespace lynx {
namespace animax {

TextContentDefault::TextContentDefault(const TextContentDataSource& data_source)
    : data_source_(data_source) {}

void TextContentDefault::Draw(Canvas& canvas, int32_t alpha) {
  auto* font_asset = data_source_.GetFontAsset();
  if (!font_asset || !font_asset->GetFont()) {
    return;
  }

  ConfigurePaint(alpha);

  DrawTextWithFont(data_source_.GetDocumentData(), *font_asset, canvas);
}

bool TextContentDefault::GetRect(RectF& out_rect) {
  out_rect.Set(0.f, 0.f, 0.f, 0.f);
  auto* font_asset = data_source_.GetFontAsset();
  if (!font_asset || !font_asset->GetFont()) {
    return false;
  }

  const auto& document_data = data_source_.GetDocumentData();
  auto font = font_asset->GetFont();
  DCHECK(font);

  float text_size = data_source_.GetTextSize();
  font->SetTextSize(text_size);

  constexpr float text_size_scale = 100.f;
  float tracking = data_source_.GetTracking() * text_size / text_size_scale;
  bool has_line = false;
  float left = 0.f, top = 0.f, right = 0.f, bottom = 0.f;

  ForEachLayoutLine(document_data, *font_asset, tracking,
                    [&](const TextSubLine& line, float x, float y) {
                      float line_left = x;
                      float line_top = y - document_data.GetSize();
                      float line_right = x + line.width_;
                      float line_bottom =
                          line_top + document_data.GetLineHeight();
                      if (!has_line) {
                        left = line_left;
                        top = line_top;
                        right = line_right;
                        bottom = line_bottom;
                        has_line = true;
                      } else {
                        left = std::min(left, line_left);
                        top = std::min(top, line_top);
                        right = std::max(right, line_right);
                        bottom = std::max(bottom, line_bottom);
                      }
                    });
  if (!has_line) {
    return false;
  }
  out_rect.Set(left, top, right, bottom);
  return true;
}

void TextContentDefault::ConfigurePaint(int32_t alpha) {
  Color fill_color = Color(data_source_.GetColor());
  fill_color.SetA(alpha * fill_color.GetA() / 255);
  fill_paint_.SetColor(fill_color);
  Color stroke_color = Color(data_source_.GetStrokeColor());
  stroke_color.SetA(alpha * stroke_color.GetA() / 255);
  stroke_paint_.SetColor(stroke_color);
  float width = data_source_.GetStrokeWidth();
  if (width > 0) {
    stroke_paint_.SetStrokeWidth(width);
  } else {
    // width zero will be treat as not visible
    stroke_paint_.SetAlpha(0);
  }
  if (layer_) {
    layer_->ApplyEffects(fill_paint_);
    layer_->ApplyEffects(stroke_paint_);
  }
}

void TextContentDefault::DrawTextWithFont(const DocumentData& document_data,
                                          FontAsset& font_asset,
                                          Canvas& canvas) {
  auto font = font_asset.GetFont();
  DCHECK(font);

  float text_size = data_source_.GetTextSize();
  font->SetTextSize(text_size);

  constexpr float text_size_scale = 100.f;
  float tracking = data_source_.GetTracking() * text_size / text_size_scale;
  ForEachLayoutLine(document_data, font_asset, tracking,
                    [&](const TextSubLine& line, float x, float y) {
                      canvas.Save();
                      canvas.Translate(x, y);
                      DrawFontTextLine(line.text_, document_data, canvas,
                                       tracking, *font);
                      canvas.Restore();
                    });
}

void TextContentDefault::ForEachLayoutLine(
    const DocumentData& document_data, FontAsset& font_asset, float tracking,
    const std::function<void(const TextSubLine&, float, float)>& visitor) {
  auto& text = document_data.GetText();
  std::vector<std::string> text_lines;
  base::SplitString(text, '\r', text_lines);
  auto text_line_count = text_lines.size();
  auto line_index = -1;
  for (size_t i = 0; i < text_line_count; i++) {
    auto& text_line = text_lines[i];
    auto* box_size = document_data.GetBoxSize();
    auto box_width =
        (box_size && !box_size->IsEmpty()) ? box_size->GetX() : 0.f;
    std::vector<TextSubLine*> lines;
    SplitGlyphTextIntoLines(text_line, box_width, font_asset, 0.f, tracking,
                            lines);
    for (auto& line : lines) {
      line_index++;
      float x = 0.f, y = 0.f;
      GetLineOffset(document_data, line_index, line->width_, x, y);
      visitor(*line, x, y);
    }
  }
}

void TextContentDefault::SplitGlyphTextIntoLines(
    const std::string& text_line, float box_width, FontAsset& font_asset,
    float font_scale, float tracking, std::vector<TextSubLine*>& text_lines) {
  auto text_line_u32 = base::U8StringToU32(text_line);
  int32_t line_count = 0;
  float current_line_width = 0;
  int32_t current_line_start_index = 0;
  int32_t current_word_start_index = 0;
  float current_word_width = 0;
  bool next_character_start_word = false;
  float space_width = 0;

  auto* chars = text_line_u32.c_str();
  for (auto i = 0; i < (int32_t)text_line_u32.length(); i++) {
    auto c = chars[i];
    float current_char_width =
        font_asset.GetFont()->MeasureText(
            base::U32StringToU8(text_line_u32.substr(i, 1))) +
        tracking;
    auto is_space = std::iswspace(c);
    if (is_space) {
      space_width = current_char_width;
      next_character_start_word = true;
    } else if (next_character_start_word) {
      next_character_start_word = false;
      current_word_start_index = i;
      current_word_width = current_char_width;
    } else {
      current_word_width += current_char_width;
    }
    current_line_width += current_char_width;

    if (box_width > 0 && current_line_width >= box_width) {
      if (is_space) {
        continue;
      }
      auto& sub_line = EnsureEnoughSubLines(++line_count);
      if (current_word_start_index == current_line_start_index) {
        const auto& sub_str = text_line_u32.substr(
            current_line_start_index, i - current_line_start_index);
        auto trimmed = TrimText(sub_str);
        float trimmed_space =
            (trimmed.length() - sub_str.length()) * space_width;
        sub_line.Set(std::move(trimmed),
                     current_line_width - current_char_width - trimmed_space);
        current_line_start_index = i;
        current_line_width = current_char_width;
        current_word_start_index = current_line_start_index;
        current_word_width = current_char_width;
      } else {
        const auto& sub_str = text_line_u32.substr(
            current_line_start_index,
            current_word_start_index - 1 - current_line_start_index);
        auto trimmed = TrimText(sub_str);
        float trimmed_space =
            (sub_str.length() - trimmed.length()) * space_width;
        sub_line.Set(std::move(trimmed), current_line_width -
                                             current_word_width -
                                             trimmed_space - space_width);
        current_line_start_index = current_word_start_index;
        current_line_width = current_word_width;
      }
    }
  }
  if (current_line_width > 0) {
    auto& line = EnsureEnoughSubLines(++line_count);
    line.Set(text_line_u32.substr(current_line_start_index),
             current_line_width);
  }

  for (auto i = 0; i < line_count; i++) {
    text_lines.push_back(text_sub_lines_[i].get());
  }
}

void TextContentDefault::GetLineOffset(const DocumentData& document_data,
                                       int32_t line_index, float line_width,
                                       float& out_x, float& out_y) {
  auto* position = document_data.GetBoxPosition();
  auto* size = document_data.GetBoxSize();
  auto baseline_shift = document_data.GetBaselineShift();

  float line_start_y = (position && !position->IsEmpty())
                           ? (document_data.GetSize() + position->GetY())
                           : 0.f;
  float line_offset = (line_index * document_data.GetLineHeight()) +
                      line_start_y - baseline_shift;

  float line_start =
      (position && !position->IsEmpty()) ? position->GetX() : 0.f;
  float box_width = (size && !size->IsEmpty()) ? size->GetX() : 0.f;
  switch (document_data.GetJustification()) {
    case DocumentJustification::kRightAlign:
      out_x = line_start + box_width - line_width;
      break;
    case DocumentJustification::kCenter:
      out_x = line_start + box_width / 2.0f - line_width / 2.0f;
      break;
    default:
      out_x = line_start;
      break;
  }
  out_y = line_offset;
}

TextContentDefault::TextSubLine& TextContentDefault::EnsureEnoughSubLines(
    int32_t num_lines) {
  for (auto i = (int32_t)text_sub_lines_.size(); i < num_lines; i++) {
    text_sub_lines_.emplace_back(
        std::unique_ptr<TextSubLine>(new TextSubLine()));
  }
  return *text_sub_lines_[num_lines - 1];
}

void TextContentDefault::DrawFontTextLine(const std::u32string& text,
                                          const DocumentData& document_data,
                                          Canvas& canvas, float tracking,
                                          Font& font) {
  if (document_data.GetStrokeOverfill()) {
    DrawCharacter(text, fill_paint_, canvas, font);
    // DrawCharacter(text, *stroke_paint_, canvas, font);
  } else {
    // DrawCharacter(text, *stroke_paint_, canvas, font);
    DrawCharacter(text, fill_paint_, canvas, font);
  }
}

void TextContentDefault::DrawCharacter(const std::u32string& text, Paint& paint,
                                       Canvas& canvas, Font& font) {
  canvas.DrawText(base::U32StringToU8(text), 0, 0, font, paint);
}

std::u32string TextContentDefault::TrimText(const std::u32string& s) {
  std::u32string::const_iterator it = s.begin();
  while (it != s.end() && std::iswspace(*it)) {
    it++;
  }

  std::u32string::const_reverse_iterator rit = s.rbegin();
  while (rit.base() != it && std::iswspace(*rit)) {
    rit++;
  }

  return std::u32string(it, rit.base());
}

}  // namespace animax
}  // namespace lynx
