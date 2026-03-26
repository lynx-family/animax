// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.loader;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;

import android.graphics.Bitmap;
import android.graphics.Color;
import com.facebook.common.references.CloseableReference;
import com.facebook.imagepipeline.bitmaps.SimpleBitmapReleaser;
import com.facebook.imagepipeline.image.CloseableStaticBitmap;
import com.facebook.imagepipeline.image.ImmutableQualityInfo;
import com.facebook.imagepipeline.image.QualityInfo;
import org.junit.Test;

public class FrescoCloseableBitmapReferenceTest {
  private CloseableReference<Bitmap> createCloseableReference() {
    Bitmap bitmap = Bitmap.createBitmap(1, 1, Bitmap.Config.ARGB_8888);
    bitmap.setPixel(0, 0, Color.argb(255, 255, 255, 255));
    QualityInfo qualityInfo = ImmutableQualityInfo.of(0, false, false);
    CloseableStaticBitmap closeableStaticBitmap =
        new CloseableStaticBitmap(bitmap, SimpleBitmapReleaser.getInstance(), qualityInfo, 0);
    return closeableStaticBitmap.convertToBitmapReference();
  }

  @Test
  public void testConstructor() {
    FrescoCloseableBitmapReference reference =
        new FrescoCloseableBitmapReference(createCloseableReference());
    assertNotNull(reference);
  }

  @Test
  public void testGet() {
    CloseableReference<Bitmap> bitmapCloseableReference = createCloseableReference();
    FrescoCloseableBitmapReference reference =
        new FrescoCloseableBitmapReference(bitmapCloseableReference);
    Bitmap bitmap = reference.get();
    assertSame(bitmap, bitmapCloseableReference.get());
  }

  @Test
  public void testIsValid() {
    CloseableReference<Bitmap> bitmapCloseableReference = createCloseableReference();
    FrescoCloseableBitmapReference reference =
        new FrescoCloseableBitmapReference(bitmapCloseableReference);
    bitmapCloseableReference.close();
    assertTrue(reference.isValid());
  }

  @Test
  public void testClose() {
    CloseableReference<Bitmap> bitmapCloseableReference = createCloseableReference();
    Bitmap bitmap = bitmapCloseableReference.get();
    FrescoCloseableBitmapReference reference =
        new FrescoCloseableBitmapReference(bitmapCloseableReference);
    bitmapCloseableReference.close();
    reference.close();
    assertTrue(bitmap.isRecycled());
  }
}
