// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.graphics.PorterDuff
import android.os.Bundle
import android.widget.Button
import android.widget.LinearLayout
import androidx.appcompat.app.AppCompatActivity
import com.lynx.animax.listener.AnimaXParam
import com.lynx.animax.listener.AnimationListenerAdapter
import com.lynx.animax.property.AnimaXFrameInfo
import com.lynx.animax.property.AnimaXKeyPath
import com.lynx.animax.property.LayerPropertyType
import com.lynx.animax.property.AnimaXPropertyCallback
import com.lynx.animax.property.AnimaXValueCallback
import com.lynx.animax.property.AnimaXValueParam
import com.lynx.animax.ui.AnimaXView
import com.lynx.animax.util.AnimaXLog
import kotlin.math.*

class LayerPropertyCallbackActivity : AppCompatActivity() {

    private val TAG = "LayerPropertyCallbackActivity"
    private lateinit var mAnimaXView: AnimaXView
    private lateinit var mButtonContainer: LinearLayout
    private val mKeyPath = AnimaXKeyPath("**")
    private val mFillKeyPath = AnimaXKeyPath("**")
    private val mCallbacks = mutableListOf<Any>()

    // Property configuration data class
    data class PropertyConfig(
        val type: LayerPropertyType,
        val displayName: String,
        val keyPath: AnimaXKeyPath,
        val valueGenerator: (AnimaXFrameInfo) -> AnimaXValueParam?
    )

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_property_update_animax)

        mAnimaXView = findViewById(R.id.animation_container)
        mButtonContainer = findViewById(R.id.button_container)

        mAnimaXView.setAutoPlay(true)
        mAnimaXView.setLoop(true)
        mAnimaXView.setSrc(LottieFiles.DynamicPropertySchema)

        setupDynamicButtons()
        addResetButton()

        mAnimaXView.addAnimationListener(object : AnimationListenerAdapter() {
            override fun onReady(param: AnimaXParam?) {
                AnimaXLog.i(TAG, "Animation ready")
            }
        })
    }

    private fun setupDynamicButtons() {
        val propertyConfigs = createPropertyConfigs()

        propertyConfigs.forEach { config ->
            createPropertyButton(config)
        }
    }

    private fun createPropertyConfigs(): List<PropertyConfig> {
        return listOf(
            // Transform properties
            PropertyConfig(
                LayerPropertyType.VISIBILITY,
                "Visibility",
                mKeyPath
            ) { frameInfo ->
                val visible = if (sin(frameInfo.overallProgress * PI * 4) > 0) 1 else 0
                AnimaXValueParam.fromNumber(visible.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_OPACITY,
                "Transform Opacity",
                mKeyPath
            ) { frameInfo ->
                val opacity = abs(sin(frameInfo.overallProgress * PI)) * 100
                AnimaXValueParam.fromNumber(opacity.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_ANCHOR,
                "Transform Anchor",
                mKeyPath
            ) { frameInfo ->
                val radius = 20.0
                val x = radius * cos(frameInfo.overallProgress * 2 * PI)
                val y = radius * sin(frameInfo.overallProgress * 2 * PI)
                AnimaXValueParam.fromCoordinate(x, y)
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_POSITION,
                "Transform Position",
                mKeyPath
            ) { frameInfo ->
                val radius = 50.0
                val x = radius * cos(frameInfo.overallProgress * 2 * PI)
                val y = radius * sin(frameInfo.overallProgress * 2 * PI)
                AnimaXValueParam.fromCoordinate(x, y)
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_SCALE,
                "Transform Scale",
                mKeyPath
            ) { frameInfo ->
                val scale = 0.5 + abs(sin(frameInfo.overallProgress * PI)) * 1.0
                AnimaXValueParam.fromCoordinate(scale, scale)
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_ROTATION,
                "Transform Rotation",
                mKeyPath
            ) { frameInfo ->
                val degrees = frameInfo.overallProgress * 360
                AnimaXValueParam.fromNumber(degrees.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_SKEW,
                "Transform Skew",
                mKeyPath
            ) { frameInfo ->
                val skew = sin(frameInfo.overallProgress * 2 * PI) * 30
                AnimaXValueParam.fromNumber(skew.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_SKEW_ANGLE,
                "Transform Skew Angle",
                mKeyPath
            ) { frameInfo ->
                val skewAngle = frameInfo.overallProgress * 90
                AnimaXValueParam.fromNumber(skewAngle.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_START_OPACITY,
                "Transform Start Opacity",
                mKeyPath
            ) { frameInfo ->
                val opacity = frameInfo.overallProgress * 100
                AnimaXValueParam.fromNumber(opacity.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_END_OPACITY,
                "Transform End Opacity",
                mKeyPath
            ) { frameInfo ->
                val opacity = (1 - frameInfo.overallProgress) * 100
                AnimaXValueParam.fromNumber(opacity.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_ROTATION_X,
                "Transform Rotation X",
                mKeyPath
            ) { frameInfo ->
                val degrees = sin(frameInfo.overallProgress * PI) * 180
                AnimaXValueParam.fromNumber(degrees.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_ROTATION_Y,
                "Transform Rotation Y",
                mKeyPath
            ) { frameInfo ->
                val degrees = cos(frameInfo.overallProgress * PI) * 180
                AnimaXValueParam.fromNumber(degrees.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.TRANSFORM_ROTATION_Z,
                "Transform Rotation Z",
                mKeyPath
            ) { frameInfo ->
                val degrees = frameInfo.overallProgress * 360
                AnimaXValueParam.fromNumber(degrees.toDouble())
            },

            // Text properties
            PropertyConfig(
                LayerPropertyType.TEXT_VALUE,
                "Text Value",
                mKeyPath
            ) { frameInfo ->
                val progress = (frameInfo.overallProgress * 100).toInt()
                AnimaXValueParam.fromString("Progress: $progress%")
            },

            PropertyConfig(
                LayerPropertyType.TEXT_SIZE,
                "Text Size",
                mKeyPath
            ) { frameInfo ->
                val size = 50 + abs(sin(frameInfo.overallProgress * PI)) * 24
                AnimaXValueParam.fromNumber(size.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.TEXT_COLOR,
                "Text Color",
                mKeyPath
            ) { frameInfo ->
                generateColorValue(frameInfo)
            },

            PropertyConfig(
                LayerPropertyType.TEXT_TRACKING,
                "Text Tracking",
                mKeyPath
            ) { frameInfo ->
                val tracking = sin(frameInfo.overallProgress * 2 * PI) * 10
                AnimaXValueParam.fromNumber(tracking.toDouble())
            },

            // Color and visual properties
            PropertyConfig(
                LayerPropertyType.COLOR,
                "Color",
                mFillKeyPath
            ) { frameInfo ->
                generateColorValue(frameInfo)
            },

            PropertyConfig(
                LayerPropertyType.COLOR_FILTER,
                "Color Filter",
                mFillKeyPath
            ) { frameInfo ->
                generateColorFilterValue(frameInfo)
            },

            PropertyConfig(
                LayerPropertyType.STROKE_COLOR,
                "Stroke Color",
                mFillKeyPath
            ) { frameInfo ->
                generateColorValue(frameInfo)
            },

            PropertyConfig(
                LayerPropertyType.STROKE_WIDTH,
                "Stroke Width",
                mFillKeyPath
            ) { frameInfo ->
                val width = 1 + abs(sin(frameInfo.overallProgress * PI)) * 10
                AnimaXValueParam.fromNumber(width.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.OPACITY,
                "Content Opacity",
                mFillKeyPath
            ) { frameInfo ->
                val opacity = abs(sin(frameInfo.overallProgress * PI)) * 100
                AnimaXValueParam.fromNumber(opacity.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.BLUR_RADIUS,
                "Blur Radius",
                mFillKeyPath
            ) { frameInfo ->
                val radius = abs(sin(frameInfo.overallProgress * PI)) * 20
                AnimaXValueParam.fromNumber(radius.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.ELLIPSE_SIZE,
                "Ellipse Size",
                mFillKeyPath
            ) { frameInfo ->
                val size = 50 + abs(sin(frameInfo.overallProgress * PI)) * 100
                AnimaXValueParam.fromCoordinate(size, size)
            },

            PropertyConfig(
                LayerPropertyType.RECTANGLE_SIZE,
                "Rectangle Size",
                mFillKeyPath
            ) { frameInfo ->
                val width = 50 + abs(sin(frameInfo.overallProgress * PI)) * 100
                val height = 50 + abs(cos(frameInfo.overallProgress * PI)) * 100
                AnimaXValueParam.fromCoordinate(width, height)
            },

            PropertyConfig(
                LayerPropertyType.CORNER_RADIUS,
                "Corner Radius",
                mFillKeyPath
            ) { frameInfo ->
                val radius = abs(sin(frameInfo.overallProgress * PI)) * 50
                AnimaXValueParam.fromNumber(radius.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.POSITION,
                "Content Position",
                mFillKeyPath
            ) { frameInfo ->
                val radius = 30.0
                val x = radius * cos(frameInfo.overallProgress * 2 * PI)
                val y = radius * sin(frameInfo.overallProgress * 2 * PI)
                AnimaXValueParam.fromCoordinate(x, y)
            },

            PropertyConfig(
                LayerPropertyType.REPEATER_COPIES,
                "Repeater Copies",
                mFillKeyPath
            ) { frameInfo ->
                val copies = 1 + (frameInfo.overallProgress * 5).toInt()
                AnimaXValueParam.fromNumber(copies.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.REPEATER_OFFSET,
                "Repeater Offset",
                mFillKeyPath
            ) { frameInfo ->
                val offset = (frameInfo.overallProgress * 100).toDouble()
                AnimaXValueParam.fromCoordinate(offset, offset)
            },

            // PolyStar properties
            PropertyConfig(
                LayerPropertyType.POLYSTAR_POINTS,
                "PolyStar Points",
                mFillKeyPath
            ) { frameInfo ->
                val points = 3 + (frameInfo.overallProgress * 5).toInt()
                AnimaXValueParam.fromNumber(points.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.POLYSTAR_ROTATION,
                "PolyStar Rotation",
                mFillKeyPath
            ) { frameInfo ->
                val rotation = frameInfo.overallProgress * 360
                AnimaXValueParam.fromNumber(rotation.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.POLYSTAR_INNER_RADIUS,
                "PolyStar Inner Radius",
                mFillKeyPath
            ) { frameInfo ->
                val radius = 10 + abs(sin(frameInfo.overallProgress * PI)) * 40
                AnimaXValueParam.fromNumber(radius.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.POLYSTAR_OUTER_RADIUS,
                "PolyStar Outer Radius",
                mFillKeyPath
            ) { frameInfo ->
                val radius = 30 + abs(cos(frameInfo.overallProgress * PI)) * 50
                AnimaXValueParam.fromNumber(radius.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.POLYSTAR_INNER_ROUNDED,
                "PolyStar Inner Rounded",
                mFillKeyPath
            ) { frameInfo ->
                val rounded = abs(sin(frameInfo.overallProgress * PI)) * 20
                AnimaXValueParam.fromNumber(rounded.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.POLYSTAR_OUTER_ROUNDED,
                "PolyStar Outer Rounded",
                mFillKeyPath
            ) { frameInfo ->
                val rounded = abs(cos(frameInfo.overallProgress * PI)) * 20
                AnimaXValueParam.fromNumber(rounded.toDouble())
            },

            // Drop Shadow properties
            PropertyConfig(
                LayerPropertyType.DROP_SHADOW_COLOR,
                "Drop Shadow Color",
                mFillKeyPath
            ) { frameInfo ->
                generateColorValue(frameInfo)
            },

            PropertyConfig(
                LayerPropertyType.DROP_SHADOW_OPACITY,
                "Drop Shadow Opacity",
                mFillKeyPath
            ) { frameInfo ->
                val opacity = abs(sin(frameInfo.overallProgress * PI)) * 100
                AnimaXValueParam.fromNumber(opacity.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.DROP_SHADOW_DIRECTION,
                "Drop Shadow Direction",
                mFillKeyPath
            ) { frameInfo ->
                val direction = frameInfo.overallProgress * 360
                AnimaXValueParam.fromNumber(direction.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.DROP_SHADOW_DISTANCE,
                "Drop Shadow Distance",
                mFillKeyPath
            ) { frameInfo ->
                val distance = abs(sin(frameInfo.overallProgress * PI)) * 50
                AnimaXValueParam.fromNumber(distance.toDouble())
            },

            PropertyConfig(
                LayerPropertyType.DROP_SHADOW_RADIUS,
                "Drop Shadow Radius",
                mFillKeyPath
            ) { frameInfo ->
                val radius = abs(cos(frameInfo.overallProgress * PI)) * 20
                AnimaXValueParam.fromNumber(radius.toDouble())
            }
        )
    }

    private fun generateColorValue(frameInfo: AnimaXFrameInfo): AnimaXValueParam {
        val progress = frameInfo.overallProgress
        return when {
            progress < 0.2 -> AnimaXValueParam.fromColor(0xffff0000.toInt()) // Red
            progress < 0.4 -> AnimaXValueParam.fromColor(0xff00ff00.toInt()) // Green
            progress < 0.6 -> AnimaXValueParam.fromColor(0xff0000ff.toInt()) // Blue
            progress < 0.8 -> AnimaXValueParam.fromColor(0xffff00ff.toInt()) // Purple
            else -> AnimaXValueParam.fromColor(0xffffa500.toInt()) // Orange
        }
    }

    private fun generateColorFilterValue(frameInfo: AnimaXFrameInfo): AnimaXValueParam {
        val progress = frameInfo.overallProgress
        return when {
            progress < 0.125 ->
                AnimaXValueParam.fromColorFilter(0x00000000, PorterDuff.Mode.SRC_OVER.ordinal)

            progress < 0.25 ->
                AnimaXValueParam.fromColorFilter(
                    0xFF0000FF.toInt(),
                    PorterDuff.Mode.MULTIPLY.ordinal
                )

            progress < 0.375 ->
                AnimaXValueParam.fromColorFilter(0x80FFFFFF.toInt(), PorterDuff.Mode.SCREEN.ordinal)

            progress < 0.5 ->
                AnimaXValueParam.fromColorFilter(
                    0x80000000.toInt(),
                    PorterDuff.Mode.MULTIPLY.ordinal
                )

            progress < 0.75 ->
                AnimaXValueParam.fromColorFilter(
                    0xFF00FF00.toInt(),
                    PorterDuff.Mode.OVERLAY.ordinal
                )

            progress < 0.875 ->
                AnimaXValueParam.fromColorFilter(0xCC0080FF.toInt(), PorterDuff.Mode.DST.ordinal)

            else ->
                AnimaXValueParam.fromColorFilter(0x80FFFF00.toInt(), PorterDuff.Mode.SRC.ordinal)
        }
    }

    private fun createPropertyButton(config: PropertyConfig) {
        val button = Button(this).apply {
            text = config.displayName
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = 8
            }

            setOnClickListener {
                addPropertyCallback(config)
            }
        }

        mButtonContainer.addView(button)
    }

    private fun addPropertyCallback(config: PropertyConfig) {
        val callback = object : AnimaXValueCallback {
            override fun getValue(frameInfo: AnimaXFrameInfo): AnimaXValueParam? {
                return config.valueGenerator(frameInfo)
            }
        }

        mAnimaXView.player.addLayerPropertyCallback(
            config.type,
            config.keyPath,
            callback,
            createPropertyCallback(config.displayName)
        )
        mCallbacks.add(callback)
    }

    private fun addResetButton() {
        val resetButton = Button(this).apply {
            text = "Reset All Callbacks"
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                topMargin = 24
            }

            setOnClickListener {
                resetCallbacks()
            }
        }

        mButtonContainer.addView(resetButton)
    }

    private fun resetCallbacks() {
        mCallbacks.clear()
        AnimaXLog.i(TAG, "All callbacks reset")
    }

    private fun createPropertyCallback(propertyName: String): AnimaXPropertyCallback {
        return object : AnimaXPropertyCallback {
            override fun onSuccess() {
                AnimaXLog.i(TAG, "$propertyName callback added successfully")
            }

            override fun onError(errorMessageList: List<String>) {
                AnimaXLog.e(TAG, "$propertyName callback error: $errorMessageList")
            }
        }
    }
}