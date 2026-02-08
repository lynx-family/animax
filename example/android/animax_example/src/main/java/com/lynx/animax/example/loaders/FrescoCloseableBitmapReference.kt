// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example.loaders

import android.graphics.Bitmap
import com.facebook.common.references.CloseableReference
import com.lynx.animax.loader.IAnimaXCloseableBitmapReference

/**
 * A Fresco-backed implementation of IAnimaXCloseableBitmapReference.
 * This class wraps a Fresco CloseableReference to manage bitmap lifecycle.
 */
class FrescoCloseableBitmapReference(
    bitmapCloseableReference: CloseableReference<Bitmap>
) : IAnimaXCloseableBitmapReference {

    private val mBitmapCloseableReference: CloseableReference<Bitmap> = bitmapCloseableReference.clone()

    override fun get(): Bitmap? {
        return if (mBitmapCloseableReference.isValid) {
            mBitmapCloseableReference.get()
        } else {
            null
        }
    }

    override fun close() {
        mBitmapCloseableReference.close()
    }

    override fun isValid(): Boolean {
        return mBitmapCloseableReference.isValid
    }
}
