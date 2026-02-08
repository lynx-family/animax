// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example.previewer

import android.content.Context
import com.lynx.animax.example.LottieFiles
import com.lynx.animax.util.AnimaXLog

/**
 * Manages the animation playlist for the previewer.
 * Handles loading animations from assets and navigation between them.
 */
class AnimationPlaylist {

    companion object {
        private const val TAG = "AnimationPlaylist"
    }

    private var animations: List<String> = emptyList()
    private var currentIndex = 0

    /**
     * Load animations from assets.
     * @param context the Android context for accessing assets
     * @return true if animations were loaded successfully
     */
    fun loadAnimations(context: Context): Boolean {
        animations = LottieFiles.getDownloadedAnimations(context)
        AnimaXLog.i(TAG, "Loaded ${animations.size} animations")
        return animations.isNotEmpty()
    }

    /**
     * Navigate to the next animation with circular wrapping.
     * @return the new animation path, or null if list is empty
     */
    fun next(): String? {
        if (animations.isEmpty()) return null
        currentIndex = (currentIndex + 1) % animations.size
        return currentAnimation
    }

    /**
     * Navigate to the previous animation with circular wrapping.
     * @return the new animation path, or null if list is empty
     */
    fun previous(): String? {
        if (animations.isEmpty()) return null
        currentIndex = (currentIndex - 1 + animations.size) % animations.size
        return currentAnimation
    }

    /**
     * Navigate to a specific animation index.
     * @param index the target index (will be clamped to valid range)
     * @return the animation path at the new index, or null if list is empty
     */
    fun goTo(index: Int): String? {
        if (animations.isEmpty()) return null
        currentIndex = index.coerceIn(0, animations.size - 1)
        return currentAnimation
    }

    /**
     * Get the current animation path.
     */
    val currentAnimation: String?
        get() = animations.getOrNull(currentIndex)

    /**
     * Get the current animation file name (without path).
     */
    val currentFileName: String
        get() = currentAnimation?.let { LottieFiles.getFileName(it) } ?: ""

    /**
     * Get the URI for the current animation.
     */
    val currentAnimationUri: String
        get() = currentAnimation?.let { LottieFiles.getAnimationUri(it) } ?: ""

    /**
     * Get all animation file names.
     */
    val animationNames: List<String>
        get() = animations.map { LottieFiles.getFileName(it) }

    /**
     * Get the current animation index.
     */
    val index: Int
        get() = currentIndex

    /**
     * Get the total number of animations.
     */
    val size: Int
        get() = animations.size

    /**
     * Check if the playlist is empty.
     */
    val isEmpty: Boolean
        get() = animations.isEmpty()
}
