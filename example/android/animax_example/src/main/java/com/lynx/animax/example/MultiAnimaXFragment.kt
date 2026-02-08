// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.os.Bundle
import android.util.TypedValue
import android.view.Gravity
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.GridLayout
import android.widget.TextView
import androidx.fragment.app.Fragment
import com.lynx.animax.composition.AnimaXComposition
import com.lynx.animax.composition.AnimaXCompositionFactory
import com.lynx.animax.composition.AnimaXCompositionListener
import com.lynx.animax.listener.AnimaXFPSParam
import com.lynx.animax.listener.AnimationListenerAdapter
import com.lynx.animax.ui.AnimaXView
import com.lynx.animax.util.AnimaXLog

class MultiAnimaXFragment : Fragment() {
    private val TAG = "MultiAnimaXFragment"
    private var animationContainer: GridLayout? = null
    private val animaXViews = mutableListOf<AnimaXView>()
    private val fpsTextViews = mutableListOf<TextView>()

    companion object {
        // Minimum item size in dp
        private const val MIN_ITEM_WIDTH_DP = 80
        private const val MIN_ITEM_HEIGHT_DP = 80
        private const val FPS_LABEL_HEIGHT_DP = 20
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        return inflater.inflate(R.layout.fragment_multi_animax, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        animationContainer = view.findViewById(R.id.animation_container)
        loadAnimations()
    }

    private fun dpToPx(dp: Int): Int {
        return TypedValue.applyDimension(
            TypedValue.COMPLEX_UNIT_DIP,
            dp.toFloat(),
            resources.displayMetrics
        ).toInt()
    }

    private fun loadAnimations() {
        AnimaXCompositionFactory.inst().loadUri(LottieFiles.SimpleShapeSchema, object: AnimaXCompositionListener {
            override fun onCompositionReady(composition: AnimaXComposition) {
                AnimaXLog.i(TAG, "onCompositionReady startFrame: ${composition.startFrame}, endFrame: ${composition.endFrame}")
                
                activity?.runOnUiThread {
                    animationContainer?.removeAllViews()
                    animaXViews.clear()
                    fpsTextViews.clear()
                    
                    val displayMetrics = resources.displayMetrics
                    val screenWidth = displayMetrics.widthPixels
                    val screenHeight = displayMetrics.heightPixels
                    
                    // Calculate minimum sizes in pixels
                    val minItemWidthPx = dpToPx(MIN_ITEM_WIDTH_DP)
                    val minItemHeightPx = dpToPx(MIN_ITEM_HEIGHT_DP)
                    val fpsLabelHeightPx = dpToPx(FPS_LABEL_HEIGHT_DP)
                    
                    // Calculate how many columns and rows can fit
                    val cols = (screenWidth / minItemWidthPx).coerceAtLeast(1)
                    val itemTotalHeight = minItemHeightPx + fpsLabelHeightPx
                    val rows = (screenHeight / itemTotalHeight).coerceAtLeast(1)
                    
                    // Calculate actual item dimensions to fill the screen
                    val itemWidth = screenWidth / cols
                    val itemHeight = (screenHeight / rows) - fpsLabelHeightPx

                    AnimaXLog.i(TAG, "Grid: ${cols}x${rows}, itemSize: ${itemWidth}x${itemHeight}")

                    animationContainer?.columnCount = cols
                    animationContainer?.rowCount = rows * 2 // Each cell has AnimaXView + FPS label

                    for (row in 0 until rows) {
                        for (col in 0 until cols) {
                            // Create FPS TextView
                            val fpsTextView = TextView(requireContext()).apply {
                                text = "FPS(--)"
                                textSize = 10f
                                setTextColor(0xFF666666.toInt())
                                gravity = Gravity.CENTER
                                setSingleLine(true)
                                layoutParams = GridLayout.LayoutParams(
                                    GridLayout.spec(row * 2 + 1),
                                    GridLayout.spec(col)
                                ).apply {
                                    width = itemWidth
                                    height = fpsLabelHeightPx
                                }
                            }

                            // Create AnimaXView
                            val animaxView = AnimaXView(requireContext()).apply {
                                layoutParams = GridLayout.LayoutParams(
                                    GridLayout.spec(row * 2),
                                    GridLayout.spec(col)
                                ).apply {
                                    width = itemWidth
                                    height = itemHeight
                                }
                                setLoop(true)
                                setAutoPlay(true)
                                setFpsEventInterval(1000)
                                addAnimationListener(object: AnimationListenerAdapter() {
                                    override fun onFPS(param: AnimaXFPSParam?) {
                                        val fps = param?.fps?.toInt() ?: 0
                                        activity?.runOnUiThread {
                                            fpsTextView.text = "FPS($fps)"
                                        }
                                    }
                                })
                                setComposition(composition)
                            }
                            
                            animaXViews.add(animaxView)
                            fpsTextViews.add(fpsTextView)
                            animationContainer?.addView(animaxView)
                            animationContainer?.addView(fpsTextView)
                        }
                    }
                }
            }

            override fun onCompositionFailed(errorMessage: String) {
                AnimaXLog.i(TAG, "onCompositionFailed errorMessage: $errorMessage")
            }
        })
    }

    override fun onDestroyView() {
        super.onDestroyView()
        animaXViews.forEach { it.release() }
        animaXViews.clear()
        fpsTextViews.clear()
        animationContainer = null
    }
}
