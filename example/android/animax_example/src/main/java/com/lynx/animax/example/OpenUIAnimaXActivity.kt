// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.lynx.animax.ui.AnimaXView
import com.lynx.animax.util.AnimaXLog
import com.lynx.lottieadapter.LottieAdapterView

class OpenUIAnimaXActivity : AppCompatActivity() {

    val TAG = "OpenUIAnimaXActivity"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_feed_animax)

        val animationContainer = findViewById<LottieAdapterView>(R.id.animation_container)
        animationContainer.apply {
            setImageAssetsFolder(LottieFiles.FeedOpenUIFolder)
            setAnimation(LottieFiles.FeedOpenUI)
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        AnimaXLog.i(TAG, "onDestroy")
    }
}