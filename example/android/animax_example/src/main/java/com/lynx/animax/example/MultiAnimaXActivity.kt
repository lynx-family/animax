// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.os.Bundle
import android.widget.GridLayout
import androidx.appcompat.app.AppCompatActivity
import com.lynx.animax.composition.AnimaXComposition
import com.lynx.animax.composition.AnimaXCompositionFactory
import com.lynx.animax.composition.AnimaXCompositionListener
import com.lynx.animax.ui.AnimaXView
import com.lynx.animax.util.AnimaXLog

class MultiAnimaXActivity : AppCompatActivity() {
    private val TAG = "MultiAnimaXActivity"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_multi_animax)

        val animationContainer = findViewById<GridLayout>(R.id.animation_container)


        AnimaXCompositionFactory.inst().loadUri(LottieFiles.SimpleShapeSchema, object: AnimaXCompositionListener {
            override fun onCompositionReady(composition: AnimaXComposition) {
                AnimaXLog.i(TAG, "onCompositionReady startFrame: ${composition.startFrame}, endFrame: ${composition.endFrame}")
                
                runOnUiThread {
                    animationContainer.removeAllViews()
                    val displayMetrics = resources.displayMetrics
                    val screenWidth = displayMetrics.widthPixels
                    val screenHeight = displayMetrics.heightPixels

                    val itemWidth = screenWidth / 4
                    val itemHeight = screenHeight / 7

                    for (row in 0 until 7) {
                        for (col in 0 until 4) {
                            val animaxView = AnimaXView(this@MultiAnimaXActivity).apply {
                                layoutParams = GridLayout.LayoutParams(
                                    GridLayout.spec(row),
                                    GridLayout.spec(col)
                                ).apply {
                                    width = itemWidth
                                    height = itemHeight
                                }
                                setComposition(composition)
                                setLoop(true)
                                setAutoPlay(true)
                            }
                            animationContainer.addView(animaxView)
                        }
                    }
                }
            }

            override fun onCompositionFailed(errorMessage: String) {
                AnimaXLog.i(TAG, "onCompositionFailed errorMessage: $errorMessage")
            }
        })
    }
}