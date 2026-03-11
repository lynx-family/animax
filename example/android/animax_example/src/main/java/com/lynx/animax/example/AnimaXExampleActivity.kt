// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.content.Intent
import android.os.Bundle
import android.view.LayoutInflater
import android.view.ViewGroup
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView

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
    }

    private fun setupButtonList() {
        val recyclerView = findViewById<RecyclerView>(R.id.button_list)
        adapter = ButtonAdapter(buttonItems)
        recyclerView.layoutManager = LinearLayoutManager(this)
        recyclerView.adapter = adapter
    }

    private fun initButtons() {
        buttonItems.add(ButtonItem("MultiAnimaXScene") {
            startActivity(Intent(this, MultiAnimaXActivity::class.java))
        })
        buttonItems.add(ButtonItem("LayerPropertyCallbackScene") {
            startActivity(Intent(this, LayerPropertyCallbackActivity::class.java))
        })
        buttonItems.add(ButtonItem("UpdatePropertyScene") {
            startActivity(Intent(this, UpdatePropertyActivity::class.java))
        })
        adapter.notifyDataSetChanged()
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