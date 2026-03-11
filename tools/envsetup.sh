#!/bin/bash
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

# using posix standard commands to acquire realpath of file
posix_absolute_path() {
  if [[ ! $# -eq 1 ]];then
    echo "illegal parameters $@"
    exit 1
  fi
  cd $(dirname $1) 1>/dev/null || exit 1
  local ABSOLUTE_PATH_OF_FILE="$(pwd -P)/$(basename $1)"
  cd - 1>/dev/null || exit 1
  echo $ABSOLUTE_PATH_OF_FILE
}

lynx_envsetup() {
  local SCRIPT_ABSOLUTE_PATH="$(posix_absolute_path $1)"
  local TOOLS_ABSOLUTE_PATH="$(dirname $SCRIPT_ABSOLUTE_PATH)"
  export LYNX_DIR="$(dirname $TOOLS_ABSOLUTE_PATH)"
  export BUILDTOOLS_DIR="${LYNX_DIR}/buildtools"
  export SHARED_TOOLS_DIR="${LYNX_DIR}/tools_shared"
  export PATH="${BUILDTOOLS_DIR}/llvm/bin:${BUILDTOOLS_DIR}/gn:${BUILDTOOLS_DIR}/ninja:${SHARED_TOOLS_DIR}/gn_tools:${SHARED_TOOLS_DIR}:$PATH"
  # setup node version
  export PATH=${BUILDTOOLS_DIR}/node/bin:$PATH
  # setup corepack
  export COREPACK_HOME="${BUILDTOOLS_DIR}/corepack"

  export PATH="${LYNX_DIR}/tools_shared:$PATH"
}

function android_env_setup() {
  local SCRIPT_REAL_PATH=$(posix_absolute_path $1)
  local TOOLS_REAL_PATH=$(dirname $SCRIPT_REAL_PATH)
  # Setup android sdk.
  if [ "$ANDROID_HOME" ]; then
    ln -vsnf "$ANDROID_HOME" "$TOOLS_REAL_PATH"/android_tools/sdk
  else
    if [ "$ANDROID_SDK" ]; then
      ln -vsnf "$ANDROID_SDK" "$TOOLS_REAL_PATH"/android_tools/sdk
    else
      echo "Please setup ANDROID_HOME or ANDROID_SDK for android build."
    fi
  fi

  # Setup android ndk
  if [ "$ANDROID_NDK_21" ]; then
    ln -vsnf "$ANDROID_NDK_21" "$TOOLS_REAL_PATH"/android_tools/ndk
  else
    if [ "$ANDROID_NDK" ]; then
      ln -vsnf "$ANDROID_NDK" "$TOOLS_REAL_PATH"/android_tools/ndk
    else
      echo "Please setup ANDROID_NDK_21 or ANDROID_NDK for android build."
    fi
  fi

  download_tools_shared_if_needed

  local local_properties_file1="${LYNX_DIR}/platform/android/local.properties"
  local local_properties_file2="${LYNX_DIR}/example/android/local.properties"
  local CMAKE_DIR="${LYNX_DIR}/buildtools/cmake"
  python3 $LYNX_DIR/tools_shared/android_tools/update_local_properties.py -f $local_properties_file1 $local_properties_file2 -p ndk.dir="$ANDROID_NDK" sdk.dir="$ANDROID_SDK" cmake.dir="$CMAKE_DIR"
}

HARMONY_SDK_VERSION='6.0.0.868'

function download_tools_shared_if_needed() {
  if [ ! -d "$LYNX_DIR/tools_shared" ]; then
    $LYNX_DIR/tools/hab sync . --target tools_shared --target-only --no-history
  fi
}

function python_env_setup() {
  download_tools_shared_if_needed

  VENV_PATH=$LYNX_DIR/.venv
  python3 $LYNX_DIR/tools_shared/vpython_tools/vpython_env_setup.py --root_dir $LYNX_DIR --requirements-path $LYNX_DIR/tools/vpython_tools/requirements.txt
  source $VENV_PATH/bin/activate
}

lynx_envsetup "${BASH_SOURCE:-$0}"
android_env_setup "${BASH_SOURCE:-$0}"
python_env_setup
