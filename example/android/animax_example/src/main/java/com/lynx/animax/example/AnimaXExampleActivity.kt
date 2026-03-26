// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.app.Dialog
import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.ViewGroup
import android.widget.Button
import android.widget.Switch
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.lynx.lottieadapter.AnimationType
import com.lynx.lottieadapter.LottieAdapterFactory
import com.lynx.lottieadapter.LottieAdapterView

data class ButtonItem(
    val name: String,
    val onClick: () -> Unit
)

class AnimaXExampleActivity : AppCompatActivity() {
    private val buttonItems = mutableListOf<ButtonItem>()
    private lateinit var adapter: ButtonAdapter

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_example)

        setupButtonList()
        initButtons()

        setupSwitch()
        setupGcButton()
    }

    private fun setupGcButton() {
        val gcButton = findViewById<Button>(R.id.gc_button)
        gcButton.setOnClickListener {
            System.gc();
        }
    }

    private fun setupSwitch() {
        val animationSwitch = findViewById<Switch>(R.id.animation_switch)

        animationSwitch.setOnCheckedChangeListener { _, isChecked ->
            if(isChecked) {
                animationSwitch.text = "Lottie"
                LottieAdapterFactory.setDefaultType(AnimationType.LOTTIE)
            } else {
                animationSwitch.text = "AnimaX"
                LottieAdapterFactory.setDefaultType(AnimationType.ANIMAX)
            }
        }
    }

    private fun setupButtonList() {
        val recyclerView = findViewById<RecyclerView>(R.id.button_list)
        adapter = ButtonAdapter(buttonItems)
        recyclerView.layoutManager = LinearLayoutManager(this)
        recyclerView.adapter = adapter
    }

    private fun initButtons() {
        buttonItems.add(ButtonItem("DialogAnimaXScene") {
            showAnimaXDialog()
        })
        buttonItems.add(ButtonItem("DialogAnimaXSceneWR") {
            showAnimaXDialogWithWorkaround()
        })
        buttonItems.add(ButtonItem("ActivityAnimaXScene") {
            startActivity(Intent(this, SimpleAnimaXActivity::class.java))
        })
        buttonItems.add(ButtonItem("OpenUIAnimaXScene") {
            startActivity(Intent(this, OpenUIAnimaXActivity::class.java))
        })
        buttonItems.add(ButtonItem("MultiAnimaXScene") {
            startActivity(Intent(this, MultiAnimaXActivity::class.java))
        })
        buttonItems.add(ButtonItem("LayerPropertyCallbackScene") {
            startActivity(Intent(this, LayerPropertyCallbackActivity::class.java))
        })
        buttonItems.add(ButtonItem("UpdatePropertyScene") {
            startActivity(Intent(this, UpdatePropertyActivity::class.java))
        })
        buttonItems.add(ButtonItem("LottieAdapterScene") {
            startActivity(Intent(this, LottieAdapterActivity::class.java))
        })
        buttonItems.add(ButtonItem("ScaleTypeScene"){
            startActivity(Intent(this, ScaleTypeActivity::class.java))
        })
        adapter.notifyDataSetChanged()
    }

    private fun showAnimaXDialog() {
        val dialog = Dialog(this)
        val view = LayoutInflater.from(this).inflate(R.layout.dialog_animax, null)

        val animationContainer = view.findViewById<LottieAdapterView>(R.id.animation_container)
        animationContainer.apply {
            setAnimation(LottieFiles.SimpleShape)
        }

        view.findViewById<Button>(R.id.btn_to_blank).setOnClickListener {
            startActivity(Intent(this, BlankActivity::class.java))
        }

        dialog.setContentView(view)
        dialog.show()
    }

    private fun showAnimaXDialogWithWorkaround() {
        val dialog = Dialog(this)
        val view = LayoutInflater.from(this).inflate(R.layout.dialog_animax, null)

        val animationContainer = view.findViewById<LottieAdapterView>(R.id.animation_container)
        animationContainer.apply {
            setAnimation(LottieFiles.SimpleShape)
        }

        view.findViewById<Button>(R.id.btn_to_blank).setOnClickListener {
            startActivity(Intent(this, BlankActivity::class.java))
        }

        dialog.setContentView(view)
        dialog.show()
    }
}

class ButtonAdapter(private val items: List<ButtonItem>) :
    RecyclerView.Adapter<ButtonAdapter.ButtonViewHolder>() {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ButtonViewHolder {
        val view = LayoutInflater.from(parent.context)
            .inflate(R.layout.item_button, parent, false)
        return ButtonViewHolder(view)
    }

    override fun onBindViewHolder(holder: ButtonViewHolder, position: Int) {
        val item = items[position]
        (holder.itemView as Button).apply {
            text = item.name
            setOnClickListener { item.onClick() }
        }
    }

    override fun getItemCount() = items.size

    class ButtonViewHolder(view: android.view.View) : RecyclerView.ViewHolder(view)
}