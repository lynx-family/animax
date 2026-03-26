// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.loader;

import android.graphics.Bitmap;
import android.net.Uri;
import com.facebook.common.executors.CallerThreadExecutor;
import com.facebook.common.references.CloseableReference;
import com.facebook.datasource.DataSource;
import com.facebook.drawee.backends.pipeline.Fresco;
import com.facebook.imagepipeline.common.ImageDecodeOptions;
import com.facebook.imagepipeline.common.Priority;
import com.facebook.imagepipeline.common.ResizeOptions;
import com.facebook.imagepipeline.datasource.BaseBitmapReferenceDataSubscriber;
import com.facebook.imagepipeline.image.CloseableImage;
import com.facebook.imagepipeline.request.ImageRequest;
import com.facebook.imagepipeline.request.ImageRequestBuilder;
import com.lynx.animax.loader.AnimaXLoaderResponse;
import com.lynx.animax.loader.IAnimaXCloseableBitmapReference;
import com.lynx.animax.loader.IAnimaXLoaderCompletionHandler;
import com.lynx.animax.loader.IAnimaXLoaderRequest;
import com.lynx.animax.loader.SimpleCloseableBitmapReference;
import com.lynx.animax.util.AnimaXLog;
import com.lynx.animax.util.DeviceUtil;
import com.lynx.animax.util.UriUtil;

public class FrescoUtil {
  private static final String TAG = "FrescoUtil";
  private static final int RGBA8888_PIXEL_BYTES = 4;

  public interface FrescoBitmapCompletionHandler {
    void onComplete(IAnimaXCloseableBitmapReference bitmapReference, String errorMessage);
  }

  public static boolean tryHandleLoaderRequestWithFresco(
      IAnimaXLoaderRequest request, IAnimaXLoaderCompletionHandler completionHandler) {
    IAnimaXLoaderRequest.IImageInfo imageInfo = request.getImageInfo();
    if (imageInfo == null) {
      return false;
    }
    Uri uri = UriUtil.safeParse(request.getUri());
    FrescoUtil.fetchBitmapUriWithFresco(
        uri, imageInfo.getWidth(), imageInfo.getHeight(), (bitmap, error_message) -> {
          AnimaXLoaderResponse<?> response = null;
          if (bitmap == null || !error_message.isEmpty()) {
            response = AnimaXLoaderResponse.createErrorResponse(new Throwable(error_message));
          } else {
            response = AnimaXLoaderResponse.createBitmapResponse(bitmap);
          }
          completionHandler.onComplete(response);
        });
    return true;
  }

  public static void fetchBitmapUriWithFresco(
      Uri imageUri, int width, int height, FrescoBitmapCompletionHandler bitmapCompletionHandler) {
    ImageRequestBuilder requestBuilder =
        ImageRequestBuilder
            .newBuilderWithSource(imageUri)
            // set the ImageDecodeOptions to force static and RGBA8888 image
            // otherwise, Fresco may change the format of the image.
            .setImageDecodeOptions(ImageDecodeOptions.newBuilder()
                                       .setBitmapConfig(Bitmap.Config.ARGB_8888)
                                       .setForceStaticImage(true)
                                       .build())
            .setRequestPriority(Priority.HIGH);

    if (DeviceUtil.needSampleImage(width, height)) {
      // Sample image to half size. The value is rounded down to the nearest power of 2.
      requestBuilder.setResizeOptions(new ResizeOptions(width / 2, height / 2));
    }

    ImageRequest request = requestBuilder.build();

    DataSource<CloseableReference<CloseableImage>> dataSource =
        Fresco.getImagePipeline().fetchDecodedImage(request, TAG);

    dataSource.subscribe(new BaseBitmapReferenceDataSubscriber() {
      @Override
      protected void onNewResultImpl(CloseableReference<Bitmap> bitmapReference) {
        Bitmap bitmap = bitmapReference != null ? bitmapReference.get() : null;
        if (bitmap != null) {
          if (checkBitmapRGBA8888(bitmap)) {
            // the format of the bitmap is RGBA8888.
            bitmapCompletionHandler.onComplete(
                new FrescoCloseableBitmapReference(bitmapReference), "");
            // do not recycle the bitmap, Fresco will do it for you.
          } else {
            AnimaXLog.i(TAG,
                "Fresco returned a bitmap with an invalid format. Attempting to convert it to "
                    + "ARGB_8888.");
            // Even if we set the ImageDecodeOptions to force static and RGBA8888 image
            // Fresco may not honor our ImageDecodeOptions. Wide color gamut images may be
            // decoded as RGBA_16. We converted it back to RGBA_8888.
            Bitmap converted = bitmap.copy(Bitmap.Config.ARGB_8888, false);
            if (checkBitmapRGBA8888(converted)) {
              bitmapCompletionHandler.onComplete(new SimpleCloseableBitmapReference(converted), "");
            } else {
              // if the bitmap is still not RGBA8888 after converted, we return null.
              bitmapCompletionHandler.onComplete(null,
                  "Fresco returned unknown bitmap format which cannot be converted to RGBA8888.");
            }
          }
        } else {
          bitmapCompletionHandler.onComplete(null, "Fresco returned a null bitmap on success.");
        }
        dataSource.close();
      }

      @Override
      public void onFailureImpl(DataSource dataSource) {
        String errorMessage = "Fresco failed to load bitmap.";
        if (dataSource.getFailureCause() != null) {
          errorMessage = errorMessage + "reason: " + dataSource.getFailureCause().getMessage();
        }
        bitmapCompletionHandler.onComplete(null, errorMessage);
        dataSource.close();
      }
    }, CallerThreadExecutor.getInstance());
  }

  private static boolean checkBitmapRGBA8888(Bitmap bitmap) {
    if (bitmap == null) {
      return false;
    }
    int expectedBytes = bitmap.getWidth() * bitmap.getHeight() * RGBA8888_PIXEL_BYTES;
    int realBytes = bitmap.getByteCount();
    return expectedBytes == realBytes;
  }
}
