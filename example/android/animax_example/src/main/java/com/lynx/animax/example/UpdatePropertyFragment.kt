// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.graphics.PorterDuff
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.LinearLayout
import androidx.fragment.app.Fragment
import com.lynx.animax.listener.AnimaXParam
import com.lynx.animax.listener.AnimationListenerAdapter
import com.lynx.animax.property.AnimaXKeyPath
import com.lynx.animax.property.LayerPropertyType
import com.lynx.animax.property.AnimaXPropertyCallback
import com.lynx.animax.property.AnimaXValueParam
import com.lynx.animax.ui.AnimaXView
import com.lynx.animax.util.AnimaXLog

class UpdatePropertyFragment : Fragment() {

    private val TAG = "UpdatePropertyFragment"
    private var mAnimaXView: AnimaXView? = null
    private var mButtonContainer: LinearLayout? = null
    private val mKeyPath = AnimaXKeyPath("**")
    private val mFillKeyPath = AnimaXKeyPath("**")

    // Property configuration data class for static updates
    data class PropertyUpdateConfig(
        val type: LayerPropertyType,
        val displayName: String,
        val keyPath: AnimaXKeyPath,
        val staticValue: AnimaXValueParam
    )

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        return inflater.inflate(R.layout.fragment_property, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        mAnimaXView = view.findViewById(R.id.animation_container)
        mButtonContainer = view.findViewById(R.id.button_container)
        
        mAnimaXView?.setAutoPlay(true)
        mAnimaXView?.setLoop(true)
        mAnimaXView?.setSrc(LottieFiles.DynamicPropertySchema)

        setupStaticUpdateButtons()
        addResetButton()

        mAnimaXView?.addAnimationListener(object : AnimationListenerAdapter() {
            override fun onReady(param: AnimaXParam?) {
                AnimaXLog.i(TAG, "Animation ready")
                mAnimaXView?.player?.getKeysForKeyPath(AnimaXKeyPath("**")) { paths ->
                    AnimaXLog.i(TAG, "getKeysForKeyPath: $paths")
                }
            }
        })
    }

    private fun setupStaticUpdateButtons() {
        val propertyConfigs = createStaticPropertyConfigs()
        
        propertyConfigs.forEach { config ->
            createPropertyButton(config)
        }
    }

    private fun createStaticPropertyConfigs(): List<PropertyUpdateConfig> {
        return listOf(
            // Transform properties - static values
            PropertyUpdateConfig(
                LayerPropertyType.VISIBILITY,
                "Toggle Visibility",
                mKeyPath,
                AnimaXValueParam.fromNumber(0.0) // Hide layer
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_OPACITY,
                "Set Opacity 50%",
                mKeyPath,
                AnimaXValueParam.fromNumber(50.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_ANCHOR,
                "Set Anchor Point",
                mKeyPath,
                AnimaXValueParam.fromCoordinate(25.0, 25.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_POSITION,
                "Move Position",
                mKeyPath,
                AnimaXValueParam.fromCoordinate(50.0, 50.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_POSITION,
                "Move Position Add",
                mKeyPath,
                AnimaXValueParam.fromCoordinate(20.0, 20.0, AnimaXValueParam.ApplyMode.ADD, AnimaXValueParam.FRAME_INDEX_ALL)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_SCALE,
                "Scale 150%",
                mKeyPath,
                AnimaXValueParam.fromCoordinate(1.5, 1.5)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_ROTATION,
                "Rotate 45°",
                mKeyPath,
                AnimaXValueParam.fromNumber(45.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_ROTATION,
                "Rotate 45 (Add)°",
                mKeyPath,
                AnimaXValueParam.fromNumber(45.0, AnimaXValueParam.ApplyMode.ADD, AnimaXValueParam.FRAME_INDEX_ALL)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_SKEW,
                "Skew 15°",
                mKeyPath,
                AnimaXValueParam.fromNumber(15.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_SKEW_ANGLE,
                "Skew Angle 30°",
                mKeyPath,
                AnimaXValueParam.fromNumber(30.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_START_OPACITY,
                "Start Opacity 80%",
                mKeyPath,
                AnimaXValueParam.fromNumber(80.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_END_OPACITY,
                "End Opacity 20%",
                mKeyPath,
                AnimaXValueParam.fromNumber(20.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_ROTATION_X,
                "Rotate X 90°",
                mKeyPath,
                AnimaXValueParam.fromNumber(90.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_ROTATION_Y,
                "Rotate Y 45°",
                mKeyPath,
                AnimaXValueParam.fromNumber(45.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TRANSFORM_ROTATION_Z,
                "Rotate Z 180°",
                mKeyPath,
                AnimaXValueParam.fromNumber(180.0)
            ),

            // Text properties
            PropertyUpdateConfig(
                LayerPropertyType.TEXT_VALUE,
                "Set Text \"UPDATED\"",
                mKeyPath,
                AnimaXValueParam.fromString("UPDATED")
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TEXT_SIZE,
                "Text Size 72px",
                mKeyPath,
                AnimaXValueParam.fromNumber(72.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TEXT_COLOR,
                "Text Color Red",
                mKeyPath,
                AnimaXValueParam.fromColor(0xffff0000.toInt())
            ),

            PropertyUpdateConfig(
                LayerPropertyType.TEXT_TRACKING,
                "Text Tracking 5",
                mKeyPath,
                AnimaXValueParam.fromNumber(5.0)
            ),

            // Color and visual properties
            PropertyUpdateConfig(
                LayerPropertyType.COLOR,
                "Fill Color Blue",
                mFillKeyPath,
                AnimaXValueParam.fromColor(0xff0000ff.toInt())
            ),

            PropertyUpdateConfig(
                LayerPropertyType.COLOR_FILTER,
                "Color Filter Green",
                mFillKeyPath,
                AnimaXValueParam.fromColorFilter(0x8000FF00.toInt(), PorterDuff.Mode.MULTIPLY.ordinal)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.STROKE_COLOR,
                "Stroke Color Purple",
                mFillKeyPath,
                AnimaXValueParam.fromColor(0xff800080.toInt())
            ),

            PropertyUpdateConfig(
                LayerPropertyType.STROKE_WIDTH,
                "Stroke Width 8px",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(8.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.STROKE_WIDTH,
                "Stroke Width Add 2px",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(2.0, AnimaXValueParam.ApplyMode.ADD, AnimaXValueParam.FRAME_INDEX_ALL)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.OPACITY,
                "Content Opacity 75%",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(75.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.BLUR_RADIUS,
                "Blur Radius 10px",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(10.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.ELLIPSE_SIZE,
                "Ellipse Size 120x120",
                mFillKeyPath,
                AnimaXValueParam.fromCoordinate(120.0, 120.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.RECTANGLE_SIZE,
                "Rectangle Size 150x100",
                mFillKeyPath,
                AnimaXValueParam.fromCoordinate(150.0, 100.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.CORNER_RADIUS,
                "Corner Radius 25px",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(25.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.POSITION,
                "Content Position 60x60",
                mFillKeyPath,
                AnimaXValueParam.fromCoordinate(60.0, 60.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.REPEATER_COPIES,
                "Repeater 4 Copies",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(4.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.REPEATER_OFFSET,
                "Repeater Offset 50x50",
                mFillKeyPath,
                AnimaXValueParam.fromCoordinate(50.0, 50.0)
            ),

            // PolyStar properties
            PropertyUpdateConfig(
                LayerPropertyType.POLYSTAR_POINTS,
                "PolyStar 6 Points",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(6.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.POLYSTAR_ROTATION,
                "PolyStar Rotation 60°",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(60.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.POLYSTAR_INNER_RADIUS,
                "PolyStar Inner Radius 30px",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(30.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.POLYSTAR_OUTER_RADIUS,
                "PolyStar Outer Radius 60px",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(60.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.POLYSTAR_INNER_ROUNDED,
                "PolyStar Inner Rounded 10px",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(10.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.POLYSTAR_OUTER_ROUNDED,
                "PolyStar Outer Rounded 15px",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(15.0)
            ),

            // Drop Shadow properties
            PropertyUpdateConfig(
                LayerPropertyType.DROP_SHADOW_COLOR,
                "Shadow Color Black",
                mFillKeyPath,
                AnimaXValueParam.fromColor(0xff000000.toInt())
            ),

            PropertyUpdateConfig(
                LayerPropertyType.DROP_SHADOW_OPACITY,
                "Shadow Opacity 80%",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(80.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.DROP_SHADOW_DIRECTION,
                "Shadow Direction 135°",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(135.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.DROP_SHADOW_DISTANCE,
                "Shadow Distance 20px",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(20.0)
            ),

            PropertyUpdateConfig(
                LayerPropertyType.DROP_SHADOW_RADIUS,
                "Shadow Blur 10px",
                mFillKeyPath,
                AnimaXValueParam.fromNumber(10.0)
            )
        )
    }

    private fun createPropertyButton(config: PropertyUpdateConfig) {
        val button = Button(requireContext()).apply {
            text = config.displayName
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                bottomMargin = 8
            }
            
            setOnClickListener {
                updateProperty(config)
            }
        }
        
        mButtonContainer?.addView(button)
    }

    private fun updateProperty(config: PropertyUpdateConfig) {
        mAnimaXView?.player?.updateLayerProperty(
            config.type,
            config.keyPath,
            config.staticValue,
            createPropertyCallback(config.displayName)
        )
    }

    private fun addResetButton() {
        val resetButton = Button(requireContext()).apply {
            text = "Reset Animation"
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT
            ).apply {
                topMargin = 24
            }
            
            setOnClickListener {
                resetAnimation()
            }
        }
        
        mButtonContainer?.addView(resetButton)
    }

    private fun resetAnimation() {
        // Reload the animation to reset all properties
        mAnimaXView?.player?.reload()
        AnimaXLog.i(TAG, "Animation reset")
    }

    private fun createPropertyCallback(propertyName: String): AnimaXPropertyCallback {
        return object: AnimaXPropertyCallback {
            override fun onSuccess() {
                AnimaXLog.i(TAG, "$propertyName updated successfully")
            }

            override fun onError(errorMessageList: List<String>) {
                AnimaXLog.e(TAG, "$propertyName update error: $errorMessageList")
            }
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        mAnimaXView?.release()
        mAnimaXView = null
        mButtonContainer = null
    }
}
