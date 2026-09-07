#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

import os
import runpy
import sys


def read_ffmpeg_revision(deps_path: str) -> str:
    deps_path = os.path.realpath(deps_path)
    revision = runpy.run_path(
        deps_path,
        init_globals={'root_dir': os.path.dirname(deps_path)},
    )['FFMPEG_REVISION']
    if not isinstance(revision, str) or not revision:
        raise ValueError('FFMPEG_REVISION in DEPS must be a string')
    return revision


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise SystemExit(f'Usage: {sys.argv[0]} DEPS')
    print(read_ffmpeg_revision(sys.argv[1]))
