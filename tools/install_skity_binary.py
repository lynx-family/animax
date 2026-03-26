#!/usr/bin/env python
# Copyright 2024 The Lynx Authors. All rights reserved.
# Licensed under the Apache License Version 2.0 that can be found in the
# LICENSE file in the root directory of this source tree.

import os
import shutil
import sys

def copy_file(src, dst):

    dir_name = os.path.dirname(dst)
    if not os.path.exists(dir_name):
        os.makedirs(dir_name)

    shutil.copy(src, dst)
    # create empty dst.d file
    with open(dst + ".d", "w") as f:
        f.write("")

if __name__ == "__main__":
    # usage: python install_skity_binary.py src_path dst_path
    src_path = sys.argv[1]
    dst_path = sys.argv[2]
    copy_file(src_path, dst_path)