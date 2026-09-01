#!/usr/bin/env python3
# Copyright 2026 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

import ast
import sys


def read_ffmpeg_revision(deps_path: str) -> str:
    with open(deps_path, encoding='utf-8') as deps_file:
        deps_tree = ast.parse(deps_file.read(), filename=deps_path)

    for node in deps_tree.body:
        if not isinstance(node, ast.Assign):
            continue
        if any(
            isinstance(target, ast.Name) and target.id == 'FFMPEG_REVISION'
            for target in node.targets
        ):
            revision = ast.literal_eval(node.value)
            if not isinstance(revision, str) or not revision:
                raise ValueError('FFMPEG_REVISION in DEPS must be a string')
            return revision

    raise ValueError('FFMPEG_REVISION is missing from DEPS')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        raise SystemExit(f'Usage: {sys.argv[0]} DEPS')
    print(read_ffmpeg_revision(sys.argv[1]))
