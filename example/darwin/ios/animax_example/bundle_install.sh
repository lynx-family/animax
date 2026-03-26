# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
set -euo pipefail

root_dir=$(pwd)/../../../../../
root_dir=$(readlink -f $root_dir)
echo "root_dir: $root_dir"

source_cache_dir=$HOME/.cocoapods/spec-repo-lynx
command="pod install --verbose --repo-update"
project_name="AnimaXExample.xcodeproj"

export COCOAPODS_CONVERT_GIT_TO_HTTP=false
export LANG=en_US.UTF-8
export SDKROOT=/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk

export COCOAPODS_LOCAL_SOURCE_REPO=$source_cache_dir/.git

set -e

# Generate podspec files
gen_podspec_start_time=$(date +%s)
pushd ../../../../..
python3 tools/ios_tools/generate_podspec_scripts_by_gn.py --target //platform/darwin/ios/animax:animax_podspec
python3 tools/ios_tools/generate_podspec_scripts_by_gn.py --target //platform/darwin/ios/lottie_adapter:lottie_adapter_podspec
python3 tools/ios_tools/generate_podspec_scripts_by_gn.py --target //lynx/base/platform/darwin:lynx_base_podspec 
python3 tools/ios_tools/generate_podspec_scripts_by_gn.py --target //lynx/platform/darwin/ios/lynx_service_api:LynxServiceAPI_podspec
popd
gen_podspec_end_time=$(date +%s)

# clear cache
rm -rf $HOME/.cocoapods/spec-repo-lynx
rm -rf $HOME/.cocoapods/repos/*\.cocoapods-spec-repo
if [ -d ../../../../Gemfile.lock ]; then
  rm ../../../../Gemfile.lock
fi

# prepare source cache
prepare_source_start_time=$(date +%s)
python3 $root_dir/tools/ios_tools/prepare_cocoapods_source.py --source-list Podfile.flatten --cache-dir $source_cache_dir
prepare_source_end_time=$(date +%s)

pod_install_start_time=$(date +%s)
SDKROOT=$(xcrun --sdk macosx --show-sdk-path) bundle install

bundle exec pod deintegrate "$project_name"
rm -rf Podfile.lock
bundle exec $command
pod_install_end_time=$(date +%s)

echo "generate podspec files cost: $((gen_podspec_end_time - gen_podspec_start_time)) seconds"
echo "download pod sources cost: $((prepare_source_end_time - prepare_source_start_time)) seconds"
echo "pod install cost: $((pod_install_end_time - pod_install_start_time)) seconds"
