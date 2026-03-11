#!/usr/bin/env bash
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

set -euo pipefail

# Minimal build script for AnimaXExample Android (compile-only).
# Purpose: verify the project builds a debug APK without installation.
# Notes:
# - No install step; uses Gradle assembleDebug.
# - Output products go to animax_example/build.

# Resolve repository root from this script location
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd -P)"
ANDROID_DIR="${ROOT_DIR}/example/android"
APP_MODULE_DIR="${ANDROID_DIR}/animax_example"
BUILD_DIR="${APP_MODULE_DIR}/build"

# Source environment (build tools, SDK/NDK, Python venv)
source "${ROOT_DIR}/tools/envsetup.sh"

echo "[1/3] Cleaning previous artifacts"
rm -rf "${BUILD_DIR}" || true

echo "[2/3] Syncing external dependencies"
pushd "${ROOT_DIR}" >/dev/null
${ROOT_DIR}/tools/hab sync .
popd >/dev/null

echo "[3/3] Building (compile-only) → ${BUILD_DIR}"
pushd "${ANDROID_DIR}" >/dev/null
./gradlew :AnimaXExample:assembleNoasanDebug
popd >/dev/null

# Locate the built APK and print its absolute path.
APK_CANDIDATE="${BUILD_DIR}/outputs/apk/noasan/debug/AnimaXExample-noasan-debug.apk"
if [[ ! -f "${APK_CANDIDATE}" ]]; then
  APK_CANDIDATE=$(find "${BUILD_DIR}/outputs/apk" -type f -name '*debug*.apk' | head -n 1 || true)
fi

if [[ -n "${APK_CANDIDATE}" && -f "${APK_CANDIDATE}" ]]; then
  APK_ABS_DIR="$(cd "$(dirname "${APK_CANDIDATE}")" && pwd -P)"
  APK_ABS_PATH="${APK_ABS_DIR}/$(basename "${APK_CANDIDATE}")"
  echo "Build completed. APK: ${APK_ABS_PATH}"
else
  echo "Build completed. Products are in ${BUILD_DIR}/ (debug APK not found)" >&2
fi
