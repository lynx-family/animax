// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example.previewer

import android.widget.ImageButton
import android.widget.SeekBar
import com.lynx.animax.example.R
import com.lynx.animax.ui.AnimaXView
import com.lynx.animax.util.AnimaXLog

/**
 * Listener interface for playback state changes.
 */
interface PlaybackListener {
    fun onPlayStateChanged(isPlaying: Boolean)
    fun onLoopStateChanged(isLooping: Boolean)
    fun onProgressChanged(progress: Float, fromUser: Boolean)
}

/**
 * Controls animation playback: play/pause, loop, and progress.
 */
class PlaybackController(
    private val animaXView: AnimaXView?,
    private val listener: PlaybackListener? = null
) {

    companion object {
        private const val TAG = "PlaybackController"
    }

    var isPlaying = false
        private set

    var isLooping = true
        private set

    /**
     * Toggle play/pause state.
     */
    fun togglePlayPause() {
        isPlaying = !isPlaying
        if (isPlaying) {
            animaXView?.play()
        } else {
            animaXView?.pause()
        }
        listener?.onPlayStateChanged(isPlaying)
        AnimaXLog.i(TAG, "Play state: $isPlaying")
    }

    /**
     * Set playing state directly.
     */
    fun setPlaying(playing: Boolean) {
        if (isPlaying != playing) {
            isPlaying = playing
            listener?.onPlayStateChanged(isPlaying)
        }
    }

    /**
     * Toggle loop mode.
     */
    fun toggleLoop() {
        isLooping = !isLooping
        animaXView?.setLoop(isLooping)
        listener?.onLoopStateChanged(isLooping)
        AnimaXLog.i(TAG, "Loop state: $isLooping")
    }

    /**
     * Set animation progress (0.0 to 1.0).
     */
    fun setProgress(progress: Float) {
        animaXView?.setProgress(progress)
    }

    /**
     * Configure animation for auto-play and loop.
     */
    fun configureAnimation() {
        animaXView?.setLoop(isLooping)
        animaXView?.setAutoPlay(true)
        isPlaying = true
        listener?.onPlayStateChanged(isPlaying)
    }

    /**
     * Update play/pause button icon based on current state.
     */
    fun updatePlayPauseButton(button: ImageButton?) {
        button?.setImageResource(
            if (isPlaying) R.drawable.ic_pause else R.drawable.ic_play
        )
    }

    /**
     * Update loop button visual based on current state.
     */
    fun updateLoopButton(button: ImageButton?) {
        button?.alpha = if (isLooping) 1.0f else 0.5f
    }

    /**
     * Set up the seek bar listener.
     */
    fun setupSeekBar(seekBar: SeekBar?, onSeekStart: () -> Unit, onSeekEnd: () -> Unit) {
        seekBar?.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            private var wasPlaying = false

            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (fromUser) {
                    val normalizedProgress = progress / 100f
                    setProgress(normalizedProgress)
                    listener?.onProgressChanged(normalizedProgress, true)
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {
                wasPlaying = isPlaying
                if (isPlaying) {
                    animaXView?.pause()
                }
                onSeekStart()
            }

            override fun onStopTrackingTouch(seekBar: SeekBar?) {
                if (wasPlaying) {
                    animaXView?.play()
                }
                onSeekEnd()
            }
        })
    }
}
