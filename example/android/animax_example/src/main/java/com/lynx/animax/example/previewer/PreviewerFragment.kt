// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example.previewer

import android.graphics.Color
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageButton
import android.widget.SeekBar
import android.widget.TextView
import androidx.cardview.widget.CardView
import androidx.fragment.app.Fragment
import com.lynx.animax.example.R
import com.lynx.animax.listener.AnimaXParam
import com.lynx.animax.listener.AnimaXTapParam
import com.lynx.animax.listener.AnimationListenerAdapter
import com.lynx.animax.property.AnimaXKeyPath
import com.lynx.animax.property.AnimaXKeyPathListCallback
import com.lynx.animax.example.previewer.dialogs.ListPickerDialog
import com.lynx.animax.ui.AnimaXView
import com.lynx.animax.util.AnimaXLog
import yuku.ambilwarna.AmbilWarnaDialog

/**
 * AnimaX Previewer Fragment - A page for previewing and playing Lottie animations.
 *
 * Features:
 * - Animation preview area
 * - Play/Pause control
 * - Loop playback toggle
 * - Previous/Next animation navigation
 * - Frame progress control
 * - Frame count display
 *
 * This fragment delegates to specialized classes:
 * - [AnimationPlaylist] for animation list management
 * - [PlaybackController] for playback controls
 * - [OptionsManager] for bottom sheet options
 * - [EventsOverlay] for tap events display
 */
class PreviewerFragment : Fragment(), OptionsListener, PlaybackListener {

    companion object {
        private const val TAG = "PreviewerFragment"
    }

    // Views
    private var animaXView: AnimaXView? = null
    private var tvFileName: TextView? = null
    private var tvFrameCount: TextView? = null
    private var tvProgressPercent: TextView? = null
    private var seekBarProgress: SeekBar? = null
    private var btnPlayPause: ImageButton? = null
    private var btnLoop: ImageButton? = null
    private var btnPrevAnim: ImageButton? = null
    private var btnNextAnim: ImageButton? = null
    private var btnMore: ImageButton? = null
    private var animationContainer: CardView? = null

    // Background color (default is transparent to show checkerboard)
    private var currentBackgroundColor: Int = Color.TRANSPARENT

    // Animation state
    private var totalFrames = 0

    // Delegated components
    private lateinit var playlist: AnimationPlaylist
    private var playbackController: PlaybackController? = null
    private lateinit var optionsManager: OptionsManager
    private lateinit var eventsOverlay: EventsOverlay

    // Animation listener
    private val animationListener = object : AnimationListenerAdapter() {
        override fun onReady(param: AnimaXParam?) {
            totalFrames = param?.totalFrame?.toInt() ?: 0
            AnimaXLog.i(TAG, "Animation ready, totalFrames: $totalFrames")

            param?.let {
                val frames = IntArray(totalFrames + 1) { it }
                animaXView?.subscribeUpdateEvents(frames, true)
            }

            activity?.runOnUiThread {
                tvFrameCount?.text = "0/$totalFrames"
                seekBarProgress?.progress = 0
                tvProgressPercent?.text = "0%"
            }
        }

        override fun onStart(param: AnimaXParam?) {
            activity?.runOnUiThread {
                playbackController?.setPlaying(true)
                playbackController?.updatePlayPauseButton(btnPlayPause)
            }
        }

        override fun onUpdate(param: AnimaXParam?) {
            val currentFrame = param?.currentFrame?.toInt() ?: 0
            val progress = if (totalFrames > 0) currentFrame.toFloat() / totalFrames else 0f

            activity?.runOnUiThread {
                tvFrameCount?.text = "$currentFrame/$totalFrames"
                seekBarProgress?.progress = (progress * 100).toInt()
                tvProgressPercent?.text = "${(progress * 100).toInt()}%"
            }
        }

        override fun onComplete(param: AnimaXParam?) {
            if (playbackController?.isLooping == false) {
                activity?.runOnUiThread {
                    playbackController?.setPlaying(false)
                    playbackController?.updatePlayPauseButton(btnPlayPause)
                }
            }
        }

        override fun onTapLayers(param: AnimaXTapParam?) {
            if (optionsManager.eventsEnabled) {
                val layerList = param?.originParams?.get("layerList")?.toString() ?: "No layers"
                activity?.runOnUiThread {
                    eventsOverlay.updateContent(layerList)
                }
            }
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        return inflater.inflate(R.layout.fragment_previewer, container, false)
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)

        initComponents()
        initViews(view)
        setupControls()
        loadAnimations()
    }

    private fun initComponents() {
        playlist = AnimationPlaylist()
        optionsManager = OptionsManager(layoutInflater, this)
        eventsOverlay = EventsOverlay(layoutInflater)
    }

    private fun initViews(view: View) {
        animaXView = view.findViewById(R.id.animax_view)
        animationContainer = view.findViewById(R.id.animation_container)
        tvFileName = view.findViewById(R.id.tv_file_name)
        tvFrameCount = view.findViewById(R.id.tv_frame_count)
        tvProgressPercent = view.findViewById(R.id.tv_progress_percent)
        seekBarProgress = view.findViewById(R.id.seek_bar_progress)
        btnPlayPause = view.findViewById(R.id.btn_play_pause)
        btnLoop = view.findViewById(R.id.btn_loop)
        btnPrevAnim = view.findViewById(R.id.btn_prev_frame)
        btnNextAnim = view.findViewById(R.id.btn_next_frame)
        btnMore = view.findViewById(R.id.btn_more)

        // Initialize playback controller with AnimaXView
        playbackController = PlaybackController(animaXView, this)
    }

    private fun setupControls() {
        // Set up animation listener
        animaXView?.addAnimationListener(animationListener)

        // Playback controls
        btnPlayPause?.setOnClickListener {
            playbackController?.togglePlayPause()
            playbackController?.updatePlayPauseButton(btnPlayPause)
        }

        btnLoop?.setOnClickListener {
            playbackController?.toggleLoop()
            playbackController?.updateLoopButton(btnLoop)
        }

        // Navigation controls
        btnPrevAnim?.setOnClickListener {
            playlist.previous()
            loadCurrentAnimation()
        }

        btnNextAnim?.setOnClickListener {
            playlist.next()
            loadCurrentAnimation()
        }

        // More options
        btnMore?.setOnClickListener {
            view?.let { optionsManager.showOptions(it) }
        }

        // Seek bar
        playbackController?.setupSeekBar(seekBarProgress, {}, {})

        // Initial button states
        playbackController?.updateLoopButton(btnLoop)
    }

    private fun loadAnimations() {
        val ctx = context ?: return
        if (!playlist.loadAnimations(ctx)) {
            tvFileName?.text = "No animations found"
            return
        }
        loadCurrentAnimation()
    }

    private fun loadCurrentAnimation() {
        val fileName = playlist.currentFileName
        val animationUri = playlist.currentAnimationUri

        if (fileName.isEmpty()) {
            tvFileName?.text = "No animations found"
            return
        }

        tvFileName?.text = fileName

        // Reset state
        totalFrames = 0

        // Load new animation
        AnimaXLog.i(TAG, "Loading animation: $animationUri")
        animaXView?.setSrc(animationUri)

        // Configure playback
        playbackController?.configureAnimation()
        playbackController?.updatePlayPauseButton(btnPlayPause)
        animaXView?.setEnableTapLayerEvent(true)
    }

    // OptionsListener implementation
    override fun onBackgroundColorClicked() {
        showColorPicker()
    }

    override fun onAnimationListClicked() {
        ListPickerDialog.builder(requireContext())
            .setTitle("Select Animation")
            .setItems(playlist.animationNames)
            .setSelectedIndex(playlist.index)
            .setOnItemClick { index ->
                AnimaXLog.i(TAG, "Selected animation at index: $index")
                playlist.goTo(index)
                loadCurrentAnimation()
            }
            .show()
    }

    override fun onKeyPathsClicked() {
        val keyPath = AnimaXKeyPath("**")
        animaXView?.getKeysForKeyPath(keyPath, object : AnimaXKeyPathListCallback {
            override fun onCallback(keyPaths: List<AnimaXKeyPath?>) {
                val keyPathStrings = keyPaths.mapNotNull { it?.keys.toString() }

                if (keyPathStrings.isEmpty()) {
                    AnimaXLog.w(TAG, "No key paths found")
                    return
                }

                ListPickerDialog.builder(requireContext())
                    .setTitle("KeyPaths (${keyPathStrings.size})")
                    .setItems(keyPathStrings)
                    .setOnItemClick { selectedIndex ->
                        AnimaXLog.i(TAG, "Selected key path: ${keyPathStrings[selectedIndex]}")
                    }
                    .show()
            }
        })
    }

    override fun onEventsToggled(enabled: Boolean) {
        if (enabled) {
            view?.let { eventsOverlay.show(it) }
        } else {
            eventsOverlay.hide()
        }
    }

    // PlaybackListener implementation
    override fun onPlayStateChanged(isPlaying: Boolean) {
        // State is managed by PlaybackController
    }

    override fun onLoopStateChanged(isLooping: Boolean) {
        // State is managed by PlaybackController
    }

    override fun onProgressChanged(progress: Float, fromUser: Boolean) {
        // Progress is managed by PlaybackController
    }

    private fun showColorPicker() {
        val context = context ?: return

        val initialColor = if (currentBackgroundColor == Color.TRANSPARENT) Color.WHITE else currentBackgroundColor

        val colorPickerDialog = AmbilWarnaDialog(context, initialColor, object : AmbilWarnaDialog.OnAmbilWarnaListener {
            override fun onCancel(dialog: AmbilWarnaDialog?) {
                AnimaXLog.i(TAG, "Color picker cancelled")
            }

            override fun onOk(dialog: AmbilWarnaDialog?, color: Int) {
                AnimaXLog.i(TAG, "Color selected: ${String.format("#%08X", color)}")
                currentBackgroundColor = color
                animationContainer?.setCardBackgroundColor(color)
            }
        })

        colorPickerDialog.show()
    }

    override fun onDestroyView() {
        super.onDestroyView()
        eventsOverlay.hide()
        animaXView?.removeAnimationListener(animationListener)
        animaXView?.release()
        animaXView = null
        animationContainer = null
        tvFileName = null
        tvFrameCount = null
        tvProgressPercent = null
        seekBarProgress = null
        btnPlayPause = null
        btnLoop = null
        btnPrevAnim = null
        btnNextAnim = null
        btnMore = null
        playbackController = null
    }
}
