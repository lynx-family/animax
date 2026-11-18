// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.

#include "src/layer/textra/desktop/textra_register.h"

#include <cstdlib>

#include "gtest/gtest.h"
#include "src/base/thread/thread_assert.h"
#include "src/layer/text_helper.h"

namespace lynx::animax {

TEST(TextraRegisterDeathTest, RegistersTextraAndKeepsTheSameHelper) {
  // Keep the process-wide registration isolated from other text helper tests.
  EXPECT_EXIT(std::exit([] {
                ThreadAssert::Init(ThreadAssert::Type::kGPU);
                RegisterAnimaXTextraDesktop();
                auto* registered_helper = &TextHelper::Impl();
                auto font_manager = registered_helper->CreateFontAssetManager();
                const bool uses_textra =
                    font_manager->GetFontMgrCollection() != nullptr;

                RegisterAnimaXTextraDesktop();
                const bool same_helper =
                    registered_helper == &TextHelper::Impl();
                return uses_textra && same_helper ? EXIT_SUCCESS : EXIT_FAILURE;
              }()),
              testing::ExitedWithCode(EXIT_SUCCESS), "");
}

}  // namespace lynx::animax
