// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example.previewer

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import com.google.android.material.bottomsheet.BottomSheetDialog
import com.lynx.animax.example.R
import com.lynx.animax.util.AnimaXLog

/**
 * Listener interface for option selection events.
 */
interface OptionsListener {
    fun onBackgroundColorClicked()
    fun onAnimationListClicked()
    fun onKeyPathsClicked()
    fun onEventsToggled(enabled: Boolean)
}

/**
 * Manages the bottom sheet options display and interactions.
 */
class OptionsManager(
    private val layoutInflater: LayoutInflater,
    private val listener: OptionsListener
) {

    companion object {
        private const val TAG = "OptionsManager"
    }

    private var isEventsEnabled = false

    /**
     * Get the current events enabled state.
     */
    val eventsEnabled: Boolean
        get() = isEventsEnabled

    /**
     * Set the events enabled state.
     */
    fun setEventsEnabled(enabled: Boolean) {
        isEventsEnabled = enabled
    }

    /**
     * Show the options bottom sheet.
     * @param parent the parent view for obtaining context
     */
    fun showOptions(parent: View) {
        val context = parent.context ?: return

        AnimaXLog.i(TAG, "Showing options")

        // Create bottom sheet dialog
        val bottomSheetDialog = BottomSheetDialog(context)
        val bottomSheetView = layoutInflater.inflate(R.layout.bottom_sheet_options, null)
        bottomSheetDialog.setContentView(bottomSheetView)

        // Handle background color option click
        bottomSheetView.findViewById<View>(R.id.option_background_color).setOnClickListener {
            bottomSheetDialog.dismiss()
            listener.onBackgroundColorClicked()
        }

        // Handle animation list option click
        bottomSheetView.findViewById<View>(R.id.option_animation_list).setOnClickListener {
            bottomSheetDialog.dismiss()
            listener.onAnimationListClicked()
        }

        // Handle key paths option click
        bottomSheetView.findViewById<View>(R.id.option_key_paths).setOnClickListener {
            bottomSheetDialog.dismiss()
            listener.onKeyPathsClicked()
        }

        // Handle events toggle
        val eventsToggleIndicator = bottomSheetView.findViewById<View>(R.id.events_toggle_indicator)
        eventsToggleIndicator.setBackgroundResource(
            if (isEventsEnabled) R.drawable.toggle_indicator_on else R.drawable.toggle_indicator_off
        )

        bottomSheetView.findViewById<View>(R.id.option_events).setOnClickListener {
            isEventsEnabled = !isEventsEnabled
            bottomSheetDialog.dismiss()
            listener.onEventsToggled(isEventsEnabled)
        }

        bottomSheetDialog.show()
    }
}
