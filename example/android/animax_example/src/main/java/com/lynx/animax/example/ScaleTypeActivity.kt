// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.lynx.lottieadapter.LottieAdapterView

class ScaleTypeActivity : AppCompatActivity()  {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_scale_type)

        val ids = intArrayOf(
            R.id.animation_container_1,
            R.id.animation_container_2,
            R.id.animation_container_3,
            R.id.animation_container_4,
            R.id.animation_container_5,
            R.id.animation_container_6,
            R.id.animation_container_7,
            R.id.animation_container_8,
            R.id.animation_container_9,
        )
        ids.forEach { id ->
            findViewById<LottieAdapterView>(id)?.setAnimation(LottieFiles.VoicePlaying)
        }
    }
}