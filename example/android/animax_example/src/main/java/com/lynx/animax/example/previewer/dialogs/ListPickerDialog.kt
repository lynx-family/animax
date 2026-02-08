// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example.previewer.dialogs

import android.content.Context
import android.graphics.Color
import android.graphics.drawable.ColorDrawable
import com.lynx.animax.util.AnimaXLog

/**
 * A reusable list picker dialog with 1-based indexing and dividers.
 * Supports optional selection indicator for the current item.
 */
class ListPickerDialog private constructor(
    private val context: Context,
    private val title: String,
    private val items: List<String>,
    private val selectedIndex: Int,
    private val onItemClick: ((Int) -> Unit)?
) {

    companion object {
        private const val TAG = "ListPickerDialog"

        /**
         * Create a new builder for ListPickerDialog.
         */
        fun builder(context: Context): Builder = Builder(context)
    }

    /**
     * Show the dialog.
     */
    fun show() {
        if (items.isEmpty()) {
            AnimaXLog.w(TAG, "No items to show in list dialog")
            return
        }

        // Create array of items with 1-based index
        val displayItems = items.mapIndexed { index, item ->
            val displayIndex = index + 1
            if (index == selectedIndex) {
                "✓ $displayIndex. $item"
            } else {
                "   $displayIndex. $item"
            }
        }.toTypedArray()

        val builder = android.app.AlertDialog.Builder(context)
        builder.setTitle(title)
        builder.setItems(displayItems) { dialog, which ->
            onItemClick?.invoke(which)
            dialog.dismiss()
        }
        builder.setNegativeButton("Cancel") { dialog, _ ->
            dialog.dismiss()
        }

        val dialog = builder.create()
        dialog.setOnShowListener {
            // Add thicker dividers to the ListView for better visual appeal
            dialog.listView?.divider = ColorDrawable(Color.parseColor("#DDDDDD"))
            dialog.listView?.dividerHeight = 2
        }
        dialog.show()
    }

    /**
     * Builder pattern for creating ListPickerDialog.
     */
    class Builder(private val context: Context) {
        private var title: String = "Select"
        private var items: List<String> = emptyList()
        private var selectedIndex: Int = -1
        private var onItemClick: ((Int) -> Unit)? = null

        /**
         * Set the dialog title.
         */
        fun setTitle(title: String): Builder {
            this.title = title
            return this
        }

        /**
         * Set the list of items to display.
         */
        fun setItems(items: List<String>): Builder {
            this.items = items
            return this
        }

        /**
         * Set the currently selected index (will show checkmark).
         * Set to -1 for no selection.
         */
        fun setSelectedIndex(index: Int): Builder {
            this.selectedIndex = index
            return this
        }

        /**
         * Set the callback for when an item is selected.
         */
        fun setOnItemClick(listener: (Int) -> Unit): Builder {
            this.onItemClick = listener
            return this
        }

        /**
         * Build and return the dialog.
         */
        fun build(): ListPickerDialog {
            return ListPickerDialog(context, title, items, selectedIndex, onItemClick)
        }

        /**
         * Build and show the dialog.
         */
        fun show() {
            build().show()
        }
    }
}
