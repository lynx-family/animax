#!/usr/bin/env bash
# Copyright 2025 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

set -euo pipefail

root_dir=$(pwd)/../../../../
root_dir=$(readlink -f $root_dir)
echo "root_dir: $root_dir"
command="pod install --verbose --repo-update"
project_name="AnimaXExample.xcodeproj"
enable_trace=true

# Prepare CocoaPods environment
export COCOAPODS_CONVERT_GIT_TO_HTTP=false
export LANG=en_US.UTF-8
export SDKROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

usage() {
  echo "Usage: $0 [OPTIONS]"
  echo "Options:"
  echo "  -h, --help         Show this help message"
  echo "  --disable-trace    Disable trace (no perfetto)"
}

handle_options() {
  for i in "$@"; do
    case $i in
      -h|--help)
        usage
        exit 0
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

enable_trace_param=$([ "$enable_trace" = false ] && echo "--enable-trace" || echo "")

pushd "$root_dir"
gn_root_dir=$(readlink -f $root_dir)
echo "gn_root_dir: $gn_root_dir"
python3 tools/ios_tools/generate_podspec_scripts_by_gn.py --root "$gn_root_dir" $enable_trace_param --target //platform/darwin/ios:animax_podspec
python3 tools/ios_tools/generate_podspec_scripts_by_gn.py --root "$gn_root_dir" $enable_trace_param --target //third_party/lynx/base/platform/darwin:lynx_base_podspec 
python3 tools/ios_tools/generate_podspec_scripts_by_gn.py --root "$gn_root_dir" $enable_trace_param --target //third_party/lynx/platform/darwin/ios/lynx_service_api:LynxServiceAPI_podspec
popd

arch -x86_64 bundle install -V --path="$root_dir"

arch -x86_64 bundle exec pod deintegrate "$project_name"
rm -rf Podfile.lock
arch -x86_64 bundle exec $command