#!/usr/bin/env python3
# Copyright 2025 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.
import os
import sys
import subprocess
import shutil
def run(cmd, cwd=None, verbose=True):
    """Run command, optionally printing output to console."""
    print(f"Running: {' '.join(cmd)} (cwd={cwd})")
    if verbose:
        # Real-time output printing
        sys.stdout.reconfigure(line_buffering=True)
        process = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1)
        for line in process.stdout:
            print(line, end='')
        process.wait()
        if process.returncode != 0:
            sys.exit(process.returncode)
    else:
        # Silent execution, no output printed
        result = subprocess.run(cmd, cwd=cwd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if result.returncode != 0:
            sys.exit(result.returncode)
def main():
    import argparse
    parser = argparse.ArgumentParser(description="Build skity wasm library")
    parser.add_argument("skity_root", help="Path to third_party/skity")
    parser.add_argument("output_root", help="Output root for libskity.a etc")
    parser.add_argument("--quiet", action="store_true", help="Suppress output during build")
    parser.add_argument("--debug", action="store_true", help="Build in debug mode (default is release)")    
    args = parser.parse_args()
    skity_root = os.path.abspath(args.skity_root)
    output_lib_root = os.path.abspath(args.output_root)
    verbose = not args.quiet
    skity_dir = os.path.join(skity_root, "skity")
    out_dir = os.path.join(skity_dir, "out/cmake_wasm")
    built_lib_path = os.path.join(out_dir, "libskity.a")
    built_lib_path_wgx = os.path.join(out_dir, "module/wgx/libwgsl-cross.a")
    built_lib_path_json_cpp = os.path.join(out_dir, "third_party/jsoncpp/src/lib_json/libjsoncpp.a")
    built_lib_path_freetype2 = os.path.join(out_dir, "src/libfreetype2.a")
    build_type = "Debug" if args.debug else "Release"
    if os.path.exists(out_dir):
        shutil.rmtree(out_dir)
    run([
        "emcmake", "cmake", "-B", out_dir,
        "-DSKITY_EXAMPLE=OFF",
        "-DSKITY_TEST=OFF",
        "-DSKITY_CODEC_MODULE=OFF",
        "-DSKITY_IO_MODULE=OFF",
        "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
        f"-DCMAKE_BUILD_TYPE={build_type}",
    ], cwd=skity_dir, verbose=verbose)
    run(["emmake", "make", "skity"], cwd=out_dir, verbose=verbose)
    os.makedirs(output_lib_root, exist_ok=True)
    output_lib_path = os.path.join(output_lib_root, "libskity.a")
    shutil.copy2(built_lib_path, output_lib_path)
    output_lib_path_wgx = os.path.join(output_lib_root, "libwgsl-cross.a")
    shutil.copy2(built_lib_path_wgx, output_lib_path_wgx)
    output_lib_path_json_cpp = os.path.join(output_lib_root, "libjsoncpp.a")
    shutil.copy2(built_lib_path_json_cpp, output_lib_path_json_cpp)
    output_lib_path_freetype2 = os.path.join(output_lib_root, "libfreetype2.a")
    shutil.copy2(built_lib_path_freetype2, output_lib_path_freetype2)
    if verbose:
        print(f"Copied {built_lib_path} to {output_lib_path}")
        print(f"Copied {built_lib_path_wgx} to {output_lib_path_wgx}")
if __name__ == "__main__":
    main()