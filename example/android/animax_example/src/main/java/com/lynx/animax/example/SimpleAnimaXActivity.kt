// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.animation.Animator
import android.animation.Animator.AnimatorListener
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.content.ContextCompat
import com.lynx.lottieadapter.LottieAdapterView
import com.lynx.lottieadapter.lottie.LottieDrawable

class SimpleAnimaXActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_simple_animax)

        val animationContainer = findViewById<LottieAdapterView>(R.id.animation_container)
        animationContainer.apply {
            val imageDrawable = ContextCompat.getDrawable(context, R.drawable.ic_msg_blue_tts_action_new)
            setImageDrawable(imageDrawable)
            setOnClickListener {
                setAutoPlay(false)
                repeatCount = 5
                repeatMode = LottieDrawable.RESTART
                setAnimation(LottieFiles.VoicePlaying)
                playAnimation()
            }
            addAnimatorListener(object : AnimatorListener{
                override fun onAnimationStart(animation: Animator?) {
                }
                override fun onAnimationEnd(animation: Animator?) {
                    setImageDrawable(imageDrawable)
                }
                override fun onAnimationCancel(animation: Animator?) {
                }
                override fun onAnimationRepeat(animation: Animator?) {
                }
            })
        }
    }
}