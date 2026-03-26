// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.animation.ValueAnimator
import android.app.AlertDialog
import android.os.Bundle
import android.widget.ArrayAdapter
import android.widget.ImageButton
import android.widget.ListView
import android.widget.SeekBar
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.lynx.animax.util.AnimaXLog
import com.lynx.lottieadapter.LottieAdapterView
import com.lynx.lottieadapter.lottie.model.KeyPath
import android.animation.ArgbEvaluator
import android.graphics.Color
import android.graphics.ColorFilter
import android.graphics.PorterDuff
import android.graphics.PorterDuffColorFilter
import com.lynx.lottieadapter.lottie.LottieProperty
import com.lynx.lottieadapter.lottie.value.LottieFrameInfo
import com.lynx.lottieadapter.lottie.value.LottieValueCallback

class LottieAdapterActivity : AppCompatActivity() {

    private val TAG = "LottieAdapterActivity"

    private lateinit var mAnimationView: LottieAdapterView
    private lateinit var mPlayPauseButton: ImageButton
    private lateinit var mLoopButton: ImageButton
    private lateinit var mSeekBar: SeekBar
    private lateinit var mFrameText: TextView
    private lateinit var mButtonsRecyclerView: RecyclerView

    private var mIsPlaying = true
    private var mIsLooping = true

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_lottie_adapter)

        mAnimationView = findViewById(R.id.animation_view)
        mPlayPauseButton = findViewById(R.id.play_pause_button)
        mLoopButton = findViewById(R.id.loop_button)
        mSeekBar = findViewById(R.id.seek_bar)
        mFrameText = findViewById(R.id.frame_text)
        mButtonsRecyclerView = findViewById(R.id.buttons_recycler_view)

        mAnimationView.setAutoPlay(true)
        mAnimationView.setRepeatCount(ValueAnimator.INFINITE)
        mAnimationView.setImageAssetsFolder(LottieFiles.FeedOpenUIFolder)
        mAnimationView.setAnimation(LottieFiles.FeedOpenUI)

        setupPlaybackControls()
        setupButtons()
        setupAnimation()
    }

    private fun setupPlaybackControls() {
        mPlayPauseButton.setOnClickListener {
            if (mIsPlaying) {
                mAnimationView.pauseAnimation()
                updatePlayState(false)
            } else {
                mAnimationView.resumeAnimation()
                updatePlayState(true)
            }
        }

        mLoopButton.setOnClickListener {
            mIsLooping = !mIsLooping
            mAnimationView.repeatCount = if (mIsLooping) ValueAnimator.INFINITE else 0
            mLoopButton.alpha = if (mIsLooping) 1.0f else 0.5f
        }

        mSeekBar.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (fromUser) {
                    mAnimationView.progress = progress / 100f
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {}

            override fun onStopTrackingTouch(seekBar: SeekBar?) {}
        })
    }

    private fun setupButtons() {
        val buttons = listOf(
            "Show KeyPaths" to { showKeyPathsDialog() },
            "Load Asset" to { showLoadAssetDialog() },
            "Filter Callback" to { setFilterCallback() }
        )

        mButtonsRecyclerView.layoutManager = LinearLayoutManager(this, LinearLayoutManager.HORIZONTAL, false)
        mButtonsRecyclerView.adapter = ButtonsAdapter(buttons)
    }

    private fun showKeyPathsDialog() {
        val keyPaths = mAnimationView.resolveKeyPath(KeyPath("**"))
        val keyPathsStrings = keyPaths.mapIndexed { index, keyPath ->
            "${index + 1}. ${keyPath.keysArray.joinToString(", ")}"
        }

        val listView = ListView(this)
        listView.adapter = ArrayAdapter(this, android.R.layout.simple_list_item_1, keyPathsStrings)

        AlertDialog.Builder(this)
            .setTitle("KeyPaths (${keyPaths.size})")
            .setView(listView)
            .setPositiveButton("OK", null)
            .show()
    }

    private fun setFilterCallback() {
        val keyPath = KeyPath("**")
        mAnimationView.addValueCallback(
            keyPath,
            LottieProperty.COLOR_FILTER,
            object : LottieValueCallback<ColorFilter>() {
                override fun getValue(frameInfo: LottieFrameInfo<ColorFilter>): ColorFilter? {
                    val progress = frameInfo.overallProgress
                    val startColor = Color.RED
                    val endColor = Color.BLUE
                    val color = ArgbEvaluator().evaluate(progress, startColor, endColor) as Int
                    return PorterDuffColorFilter(color, PorterDuff.Mode.SRC_ATOP)
                }
            }
        )
    }

    private fun showLoadAssetDialog() {
        try {
            val assetManager = assets
            val assetFiles = assetManager.list("")?.filter {
                it.endsWith(".json")
            } ?: emptyList()

            var dialog: AlertDialog? = null
            val builder = AlertDialog.Builder(this)
                .setTitle("Load Asset")
                .setItems(assetFiles.toTypedArray()) { _, which ->
                    val assetName = assetFiles[which]
                    dialog?.dismiss()
                    loadAnimation(assetName)
                }
            dialog = builder.show()
        } catch (e: Exception) {
            AnimaXLog.e(TAG, "Error loading assets")
        }
    }

    private fun loadAnimation(assetName: String) {
        mAnimationView.cancelAnimation()
        mAnimationView.setAnimation(assetName)
        mAnimationView.playAnimation()
        updatePlayState(true)
    }

    private fun setupAnimation() {
        mSeekBar.max = 100

        mAnimationView.addAnimatorUpdateListener {
            val currentFrame = mAnimationView.frame
            val maxFrame = mAnimationView.maxFrame.toInt()
            mFrameText.text = "$currentFrame/${maxFrame}"

            val progress = currentFrame.toFloat() / maxFrame
            mSeekBar.progress = (progress * 100).toInt()
        }

        updatePlayState(true)

        mIsLooping = mAnimationView.repeatCount == ValueAnimator.INFINITE
        mLoopButton.alpha = if (mIsLooping) 1.0f else 0.5f
    }

    private fun updatePlayState(playing: Boolean) {
        mIsPlaying = playing
        if (playing) {
            mPlayPauseButton.setImageResource(android.R.drawable.ic_media_pause)
        } else {
            mPlayPauseButton.setImageResource(android.R.drawable.ic_media_play)
        }
    }
}