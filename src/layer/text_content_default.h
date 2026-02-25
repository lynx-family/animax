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

#ifndef ANIMAX_SRC_LAYER_TEXT_CONTENT_DEFAULT_H_
#define ANIMAX_SRC_LAYER_TEXT_CONTENT_DEFAULT_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "src/layer/text_content.h"
#include "src/layer/text_content_data_source.h"
#include "src/render/canvas.h"

namespace lynx {
namespace animax {

// Default text layout and rendering implementation. This provides basic text
// typesetting capabilities using the built-in font engine. For higher layout
// accuracy and multilingual rendering support, we recommend integrating Textra
// as the text layout engine.
class TextContentDefault : public TextContent {
 public:
  TextContentDefault(const TextContentDataSource& data_source);
  ~TextContentDefault() = default;

  void Draw(Canvas& canvas, int32_t alpha) override;

 private:
  struct TextSubLine {
    std::u32string text_;
    float width_ = 0;
    void Set(std::u32string text, float width) {
      text_ = std::move(text);
      width_ = width;
    }
  };
  void ConfigurePaint(int32_t alpha);
  void DrawTextWithFont(const DocumentData& document_data,
                        FontAsset& font_asset, Canvas& canvas);
  void SplitGlyphTextIntoLines(const std::string& text_line, float box_width,
                               FontAsset& font_asset, float font_scale,
                               float tracking,
                               std::vector<TextSubLine*>& text_lines);
  void OffsetCanvas(Canvas& canvas, const DocumentData& document_data,
                    int32_t line_index, float line_width);

  void DrawFontTextLine(const std::u32string& text,
                        const DocumentData& document_data, Canvas& canvas,
                        float tracking, Font& font);
  void DrawCharacter(const std::u32string& text, Paint& paint, Canvas& canvas,
                     Font& font);

  TextSubLine& EnsureEnoughSubLines(int32_t num_lines);

  std::u32string TrimText(const std::u32string& s);

  const TextContentDataSource& data_source_;
  RectF rect_;
  Matrix matrix_;
  Paint fill_paint_;
  Paint stroke_paint_;

  std::unordered_map<int32_t, std::string> code_point_cache_;
  std::vector<std::unique_ptr<TextSubLine>> text_sub_lines_;
};

}  // namespace animax
}  // namespace lynx

#endif  // ANIMAX_SRC_LAYER_TEXT_CONTENT_DEFAULT_H_
