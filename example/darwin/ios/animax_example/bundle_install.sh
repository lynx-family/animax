#!/usr/bin/env bash
# Copyright 2025 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

set -euo pipefail

root_dir=$(cd "$(pwd)/../../../../" && pwd -P)
echo "root_dir: $root_dir"
command="pod install --verbose --repo-update"
project_name="AnimaXExample.xcodeproj"
enable_trace=false

# Prepare CocoaPods environment
export COCOAPODS_CONVERT_GIT_TO_HTTP=false
export LANG=en_US.UTF-8
export SDKROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

usage() {
  echo "Usage: $0 [OPTIONS]"
  echo "Options:"
  echo "  -h, --help         Show this help message"
  echo "  --enable-trace     Enable trace/perfetto"
  echo "  --disable-trace    Disable trace/perfetto (default)"
}

handle_options() {
  for i in "$@"; do
    case $i in
      -h|--help)
        usage
        exit 0
        ;;
      --enable-trace)
        enable_trace=true
        ;;
      --disable-trace)
        enable_trace=false
        ;;
      *)
        usage
        exit 1
        ;;
    esac
  done
}

handle_options "$@"

enable_trace_param=""
if [ "$enable_trace" = true ]; then
  enable_trace_param="--enable-trace"
fi

pushd "$root_dir"
gn_root_dir=$(pwd -P)
echo "gn_root_dir: $gn_root_dir"
python3 tools/ios_tools/generate_podspec_scripts_by_gn.py --root "$gn_root_dir" $enable_trace_param --target //platform/darwin/ios:animax_podspec
popd

bundle install -V --path="$root_dir"

bundle exec pod deintegrate "$project_name"
rm -rf Podfile.lock
bundle exec $command
