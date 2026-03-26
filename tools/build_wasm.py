#!/usr/bin/env python3
# Copyright 2025 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

import argparse
import os
import subprocess
import shutil


def gn_build(root_path: str, build_type: str) -> str:
    """
    Call GN gen and ninja build
    """

    out_dir = 'out/Debug'
    is_debug = 'true'
    if build_type == 'Release':
        out_dir = 'out/Release'
        is_debug = 'false'

    gn_command = 'gn gen {} --args="is_debug={} enable_animax=true use_clang_static_analyzer=false target_os=\\\"wasm\\\" target_cpu=\\\"wasm\\\"" --export-compile-commands'.format(
        out_dir, is_debug)

    ninja_command = 'ninja -C {}'.format(out_dir)

    print('begin generate GN project:')
    print('cmd: %s' % gn_command)

    saved_path = os.getcwd()
    os.chdir(root_path)

    subprocess.check_call(gn_command, shell=True)
    subprocess.check_call(ninja_command, shell=True)

    os.chdir(saved_path)
    return out_dir


def copy_wasm(root_dir: str, out_dir: str, type: str):
    source_dir = os.path.join(root_dir, out_dir)
    
    # Destination: Library src folder (for packaging/import)
    lib_target_dir = os.path.join(root_dir, 'platform/web/public/build')
    os.makedirs(lib_target_dir, exist_ok=True)
    
    files_to_copy = [
        'animax_wasm.js',
        'animax_wasm.wasm',
    ]

    if (type == "Debug"):
        files_to_copy += [
            'animax_wasm.js.symbols',
            'animax_wasm.wasm.map',
        ]
    
    for filename in files_to_copy:
        src_path = os.path.join(source_dir, filename)
        
        # Copy to lib (only JS is needed for import, but copying all for completeness/distribution)
        dst_path_lib = os.path.join(lib_target_dir, filename)
        if os.path.exists(src_path):
             shutil.copy(src_path, dst_path_lib)
             print(f'Copied {src_path} to {dst_path_lib}')


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-t', '--type', type=str,
                        default='Debug', help='Build Type. Debug or release')

    args = parser.parse_args()

    file_path = os.path.dirname(os.path.abspath(__file__))
    root_path = os.path.join(file_path, '..')

    out_dir = gn_build(root_path, args.type)

    copy_wasm(root_path, out_dir, args.type)

    pass
