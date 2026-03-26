// Copyright 2025 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.view.LayoutInflater
import android.view.ViewGroup
import android.widget.Button
import androidx.recyclerview.widget.RecyclerView

class ButtonsAdapter(private val buttons: List<Pair<String, () -> Unit>>) :
    RecyclerView.Adapter<ButtonsAdapter.ViewHolder>() {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val button = LayoutInflater.from(parent.context)
            .inflate(R.layout.button_item, parent, false) as Button
        return ViewHolder(button)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val (text, action) = buttons[position]
        holder.button.text = text
        holder.button.setOnClickListener { action() }
    }

    override fun getItemCount() = buttons.size

    class ViewHolder(val button: Button) : RecyclerView.ViewHolder(button)
}