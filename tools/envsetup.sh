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

prepend_pythonpath_if_exists() {
  local python_path="$1"
  if [[ ! -d "$python_path" ]]; then
    return 0
  fi

  python_path="$(cd "$python_path" 1>/dev/null && pwd -P)" || return 0
  case ":${PYTHONPATH:-}:" in
    *":$python_path:"*) ;;
    *)
      if [[ -n "${PYTHONPATH:-}" ]]; then
        export PYTHONPATH="$python_path:$PYTHONPATH"
      else
        export PYTHONPATH="$python_path"
      fi
      ;;
  esac
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
  prepend_pythonpath_if_exists "${LYNX_DIR}/third_party/py_deps"
  prepend_pythonpath_if_exists "${LYNX_DIR}/third_party/lynx/third_party/py_deps"
  prepend_pythonpath_if_exists "${LYNX_DIR}/../../lynx/third_party/py_deps"
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

lynx_envsetup "${BASH_SOURCE:-$0}"
android_env_setup "${BASH_SOURCE:-$0}"
