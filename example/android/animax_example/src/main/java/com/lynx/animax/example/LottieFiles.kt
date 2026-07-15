// Copyright 2026 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
package com.lynx.animax.example

import android.content.Context
import com.lynx.animax.util.UriUtil

object LottieFiles {
    val SimpleShape = "sample/simple_shape.json"
    val SimpleShapeSchema = UriUtil.fromLocalAsset(SimpleShape)
    val DynamicProperty = "sample/dp.json"
    val DynamicPropertySchema = UriUtil.fromLocalAsset(DynamicProperty)

    private const val DOWNLOAD_DIR = ""

    /**
     * Scans the assets/download directory recursively for .json and .zip files.
     * Returns a sorted list of asset paths (relative to assets folder).
     * Exported animations use reverse natural ordering and are listed before
     * bundled samples, which keep natural ordering.
     */
    fun getDownloadedAnimations(context: Context): List<String> {
        val animations = mutableListOf<String>()
        scanAssetsRecursively(context, DOWNLOAD_DIR, animations)
        val sortedAnimations = animations.sortedWith(AlphanumericComparator())
        val (exportAnimations, bundledAnimations) =
            sortedAnimations.partition { it.startsWith("export_output/") }
        return exportAnimations.reversed() + bundledAnimations
    }

    /**
     * Gets the URI for a downloaded animation file.
     */
    fun getAnimationUri(assetPath: String): String {
        return UriUtil.fromLocalAsset(assetPath)
    }

    /**
     * Gets just the filename from an asset path.
     */
    fun getFileName(assetPath: String): String {
        return assetPath.substringAfterLast("/")
    }

    private fun scanAssetsRecursively(context: Context, path: String, result: MutableList<String>) {
        try {
            val files = context.assets.list(path) ?: return
            for (file in files) {
                val fullPath = if (path.isEmpty()) file else "$path/$file"
                // Check if it's a directory by trying to list its contents
                val subFiles = context.assets.list(fullPath)
                if (subFiles != null && subFiles.isNotEmpty()) {
                    // It's a directory, recurse
                    scanAssetsRecursively(context, fullPath, result)
                } else {
                    // It's a file, check extension
                    if (file.endsWith(".json", ignoreCase = true) || 
                        file.endsWith(".zip", ignoreCase = true)) {
                        result.add(fullPath)
                    }
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }

    /**
     * Comparator that sorts strings alphanumerically with natural number ordering.
     * e.g., "1", "2", "10" instead of "1", "10", "2"
     */
    private class AlphanumericComparator : Comparator<String> {
        override fun compare(s1: String, s2: String): Int {
            val parts1 = splitIntoChunks(s1)
            val parts2 = splitIntoChunks(s2)

            val minLength = minOf(parts1.size, parts2.size)
            for (i in 0 until minLength) {
                val chunk1 = parts1[i]
                val chunk2 = parts2[i]

                val result = if (chunk1.isNumeric() && chunk2.isNumeric()) {
                    // Compare as numbers
                    chunk1.toLong().compareTo(chunk2.toLong())
                } else {
                    // Compare as strings (case-insensitive)
                    chunk1.compareTo(chunk2, ignoreCase = true)
                }

                if (result != 0) return result
            }

            return parts1.size - parts2.size
        }

        private fun splitIntoChunks(s: String): List<String> {
            val chunks = mutableListOf<String>()
            val current = StringBuilder()
            var isDigit = false

            for (c in s) {
                val currentIsDigit = c.isDigit()
                if (current.isNotEmpty() && currentIsDigit != isDigit) {
                    chunks.add(current.toString())
                    current.clear()
                }
                current.append(c)
                isDigit = currentIsDigit
            }

            if (current.isNotEmpty()) {
                chunks.add(current.toString())
            }

            return chunks
        }

        private fun String.isNumeric(): Boolean {
            return this.all { it.isDigit() }
        }
    }
}
