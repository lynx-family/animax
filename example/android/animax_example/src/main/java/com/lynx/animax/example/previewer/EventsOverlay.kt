// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example.previewer

import android.graphics.Color
import android.graphics.drawable.ColorDrawable
import android.view.Gravity
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.PopupWindow
import android.widget.TextView
import com.lynx.animax.example.R
import com.lynx.animax.util.AnimaXLog
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

/**
 * A draggable floating overlay for displaying tap events.
 * Positioned at the right edge, 1/4 from the top of the anchor view.
 */
class EventsOverlay(private val layoutInflater: LayoutInflater) {

    companion object {
        private const val TAG = "EventsOverlay"
    }

    private var popup: PopupWindow? = null
    private var contentTextView: TextView? = null
    private var isShowing = false

    /**
     * Check if the overlay is currently showing.
     */
    val isVisible: Boolean
        get() = isShowing

    /**
     * Show the overlay anchored to the given view.
     * @param anchorView the view to anchor the overlay to
     */
    fun show(anchorView: View) {
        if (isShowing) return

        val ctx = anchorView.context

        // Inflate the floating overlay layout
        val overlayView = layoutInflater.inflate(R.layout.floating_event_overlay, null)
        contentTextView = overlayView.findViewById(R.id.tv_event_content)

        // Calculate fixed size: width = half screen width, height = half of width
        val screenWidth = anchorView.width
        val popupWidth = screenWidth / 2
        val popupHeight = popupWidth / 2

        // Create PopupWindow with fixed size
        popup = PopupWindow(
            overlayView,
            popupWidth,
            popupHeight,
            false // Not focusable so it doesn't steal touch events
        ).apply {
            isOutsideTouchable = false
            setBackgroundDrawable(ColorDrawable(Color.TRANSPARENT))
        }

        // Calculate initial position (right edge, 1/4 from top)
        anchorView.post {
            val screenHeight = anchorView.height

            // Position: right edge with margin, 1/4 from top
            val xPos = screenWidth - popupWidth - 16
            val yPos = screenHeight / 4

            // Get anchor view location on screen
            val location = IntArray(2)
            anchorView.getLocationOnScreen(location)

            popup?.showAtLocation(anchorView, Gravity.NO_GRAVITY, location[0] + xPos, location[1] + yPos)

            // Set up dragging
            setupDraggable(overlayView)
        }

        isShowing = true
        AnimaXLog.i(TAG, "Events overlay shown")
    }

    /**
     * Hide the overlay.
     */
    fun hide() {
        popup?.dismiss()
        popup = null
        contentTextView = null
        isShowing = false
        AnimaXLog.i(TAG, "Events overlay hidden")
    }

    /**
     * Update the displayed content with a timestamp.
     * @param layerInfo the layer information to display
     */
    fun updateContent(layerInfo: String) {
        val timeFormat = SimpleDateFormat("HH:mm:ss", Locale.getDefault())
        val currentTime = timeFormat.format(Date())
        contentTextView?.text = "$currentTime\n$layerInfo"
    }

    /**
     * Set up draggable behavior for the overlay.
     */
    private fun setupDraggable(overlayView: View) {
        var initialX = 0
        var initialY = 0
        var initialTouchX = 0f
        var initialTouchY = 0f

        overlayView.setOnTouchListener { _, event ->
            when (event.action) {
                MotionEvent.ACTION_DOWN -> {
                    // Get current popup position
                    val location = IntArray(2)
                    overlayView.getLocationOnScreen(location)
                    initialX = location[0]
                    initialY = location[1]
                    initialTouchX = event.rawX
                    initialTouchY = event.rawY
                    true
                }
                MotionEvent.ACTION_MOVE -> {
                    // Calculate new position
                    val dx = (event.rawX - initialTouchX).toInt()
                    val dy = (event.rawY - initialTouchY).toInt()

                    popup?.update(initialX + dx, initialY + dy, -1, -1)
                    true
                }
                else -> false
            }
        }
    }
}
