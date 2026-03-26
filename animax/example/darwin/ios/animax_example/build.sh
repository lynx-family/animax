#!/usr/bin/env bash
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

set -euo pipefail

# Minimal build script for AnimaXExample (compile-only).
# Purpose: verify the project compiles without signing or IPA export.
# Notes:
# - No archive/export; uses non-signing build to iphoneos SDK.
# - Output products go to local ./build via SYMROOT.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

# Configurable parameters (override with env vars if needed).
: "${CONFIGURATION:=Debug}"
: "${SCHEME:=AnimaXExample}"
: "${WORKSPACE:=AnimaXExample.xcworkspace}"
: "${ARCH:=arm64}"
: "${BUILD_DIR:=build}"

export DISABLE_CUSTOM_GEMS_CACHE=${DISABLE_CUSTOM_GEMS_CACHE:-0}

echo "[1/4] Cleaning previous artifacts"
rm -rf Pods Podfile.lock "${BUILD_DIR}" results_bundle* ../../../.seer/seer.yml ../../../.seer/information.yml

echo "[2/4] Xcode clean (${CONFIGURATION})"
xcodebuild clean \
  -workspace "${WORKSPACE}" \
  -scheme "${SCHEME}" \
  -configuration "${CONFIGURATION}" \
  -quiet || true

echo "[3/4] Installing bundles/pods"
./bundle_install.sh

echo "[4/4] Building (compile-only) → ${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"

# Compile only: build targets without signing or IPA export.
xcodebuild \
  -workspace "${WORKSPACE}" \
  -scheme "${SCHEME}" \
  -configuration "${CONFIGURATION}" \
  -arch "${ARCH}" \
  -sdk iphoneos \
  SYMROOT="$(pwd)/${BUILD_DIR}" \
  CODE_SIGN_IDENTITY="" \
  CODE_SIGNING_REQUIRED=NO
echo "Build completed. Products are in ${BUILD_DIR}/"
