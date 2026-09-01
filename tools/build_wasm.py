#!/usr/bin/env python3
# Copyright 2025 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

import argparse
import glob
import os
import shutil
import subprocess
import sys
import tempfile


def configure_wasm_environment(root_path: str) -> None:
    emsdk_root = os.path.join(root_path, 'buildtools', 'emsdk')
    if sys.version_info < (3, 10):
        bundled_pythons = glob.glob(
            os.path.join(emsdk_root, 'python', '*_64bit', 'bin', 'python3')
        )

        def python_version(path):
            version_dir = os.path.basename(
                os.path.dirname(os.path.dirname(path))
            )
            return tuple(int(part) for part in version_dir.split('_')[0].split('.'))

        emsdk_python = max(bundled_pythons, key=python_version, default=None)
        if (
            emsdk_python
            and os.path.isfile(emsdk_python)
            and os.path.realpath(emsdk_python) != os.path.realpath(sys.executable)
        ):
            os.execv(emsdk_python, [emsdk_python] + sys.argv)
        raise RuntimeError(
            "AnimaX Web builds require Python 3.10 or newer; run "
            "source tools/envsetup.sh first."
        )

    emscripten_root = os.path.join(emsdk_root, 'upstream', 'emscripten')
    if not os.path.isdir(emscripten_root):
        raise FileNotFoundError(
            f'Emscripten is missing at {emscripten_root}; run tools/hab sync .'
        )

    os.environ['EMSDK'] = emsdk_root
    os.environ['EM_CONFIG'] = os.path.join(emsdk_root, '.emscripten')
    os.environ['EMSDK_PYTHON'] = sys.executable
    os.environ['PATH'] = os.pathsep.join([
        os.path.join(root_path, 'buildtools', 'gn'),
        os.path.join(root_path, 'buildtools', 'ninja'),
        emscripten_root,
        emsdk_root,
        os.environ.get('PATH', ''),
    ])

    # A clean Emscripten install lazily initializes both the sysroot and the
    # external Dawn port. Prime them once before Ninja starts hundreds of
    # compiler processes so first builds cannot race while mutating the cache.
    with tempfile.TemporaryDirectory(prefix='animax-emscripten-') as temp_dir:
        source = os.path.join(temp_dir, 'empty.cc')
        output = os.path.join(temp_dir, 'empty.o')
        with open(source, 'w', encoding='utf-8') as source_file:
            source_file.write('int animax_emscripten_cache_probe;\n')
        subprocess.check_call([
            os.path.join(emscripten_root, 'em++'),
            '--use-port=emdawnwebgpu',
            '-c',
            source,
            '-o',
            output,
        ])


def gn_build(root_path: str, build_type: str, package: str) -> str:
    """
    Call GN gen and ninja build
    """

    out_dir = 'out/Debug'
    is_debug = 'true'
    if build_type == 'Release':
        out_dir = 'out/Release'
        is_debug = 'false'

    gn_command = 'gn gen {} --args="is_debug={} use_clang_static_analyzer=false target_os=\\\"wasm\\\" target_cpu=\\\"wasm\\\"" --export-compile-commands'.format(
        out_dir, is_debug)

    ninja_targets = {
        'core': ['animax_wasm'],
        'textra': ['animax_textra_wasm'],
        'all': ['animax_wasm', 'animax_textra_wasm'],
    }[package]
    ninja_command = 'ninja -C {} {}'.format(out_dir, ' '.join(ninja_targets))

    print('begin generate GN project:')
    print('cmd: %s' % gn_command)

    saved_path = os.getcwd()
    os.chdir(root_path)

    subprocess.check_call(gn_command, shell=True)
    subprocess.check_call(ninja_command, shell=True)

    os.chdir(saved_path)
    return out_dir


def copy_file_if_exists(src_path: str, dst_path: str, required: bool = False):
    if not os.path.exists(src_path):
        message = f'{src_path} does not exist'
        if required:
            raise FileNotFoundError(message)
        print(f'Skipped: {message}')
        return

    shutil.copy(src_path, dst_path)
    print(f'Copied {src_path} to {dst_path}')


def copy_core_wasm(root_dir: str, out_dir: str, build_type: str):
    source_dir = os.path.join(root_dir, out_dir)

    # Destination: Library src folder (for packaging/import)
    lib_target_dir = os.path.join(root_dir, 'platform/web/core/public/build')
    os.makedirs(lib_target_dir, exist_ok=True)

    files_to_copy = [
        'animax_wasm.js',
        'animax_wasm.wasm',
    ]

    if (build_type == "Debug"):
        files_to_copy += [
            'animax_wasm.js.symbols',
            'animax_wasm.wasm.map',
        ]

    for filename in files_to_copy:
        src_path = os.path.join(source_dir, filename)

        # Copy to lib (only JS is needed for import, but copying all for completeness/distribution)
        dst_path_lib = os.path.join(lib_target_dir, filename)
        copy_file_if_exists(
            src_path,
            dst_path_lib,
            required=filename in ('animax_wasm.js', 'animax_wasm.wasm'),
        )


def copy_textra_wasm(root_dir: str, out_dir: str, required: bool = True):
    source_dir = os.path.join(root_dir, out_dir)
    lib_target_dir = os.path.join(root_dir, 'platform/web/textra/out/lib')
    os.makedirs(lib_target_dir, exist_ok=True)

    src_path = os.path.join(source_dir, 'animax_textra.wasm')
    dst_path = os.path.join(lib_target_dir, 'animax-textra.wasm')
    copy_file_if_exists(src_path, dst_path, required=required)

    copy_file_if_exists(
        src_path + '.map',
        dst_path + '.map',
        required=False,
    )


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-t', '--type', type=str,
                        default='Debug', help='Build Type. Debug or release')
    parser.add_argument('--package', type=str, choices=['core', 'textra', 'all'],
                        default='core',
                        help='Package artifact(s) to copy after GN build')

    args = parser.parse_args()

    file_path = os.path.dirname(os.path.abspath(__file__))
    # Keep Emscripten's configured paths stable. Passing the same directory as
    # both `tools/../buildtools` and its canonical path makes Emscripten think
    # its configuration changed, which clears the sysroot during parallel
    # Ninja builds.
    root_path = os.path.realpath(os.path.join(file_path, '..'))
    configure_wasm_environment(root_path)

    out_dir = gn_build(root_path, args.type, args.package)

    if args.package in ('core', 'all'):
        copy_core_wasm(root_path, out_dir, args.type)

    if args.package in ('textra', 'all'):
        copy_textra_wasm(root_path, out_dir, required=True)

    pass
