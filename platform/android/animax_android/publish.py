#!/usr/bin/python
# -*- coding: UTF-8 -*-
import argparse
import os
import sys


def param_true_if(name, condition):
    return (" -P" + name + "=true ") if condition else ""


def param_false_if(name, condition):
    return (" -P" + name + "=false ") if condition else ""


def main():
    parser = argparse.ArgumentParser(description='parse extra parameters')
    parser.add_argument('-v', '--version', help='')
    parser.add_argument('-b', '--branch', help='')
    parser.add_argument('-c', '--commit', help='')
    parser.add_argument('--publish_info', help='')
    parser.add_argument('--lite', default=False, action='store_true')
    parser.add_argument('--cxxstatic', default=False, action='store_true')
    parser.add_argument('--enable_16kb', default=False, action='store_true')
    parser.add_argument('--disable_publish', default=False, action='store_true')

    args, unknown = parser.parse_known_args()

    version = args.version
    branch = args.branch
    commit = args.commit
    publish_info = args.publish_info
    # Check if we are building Lynx lite
    lite_build = args.lite
    # use suffix to decide how to link libc++
    cxxstatic_build = args.cxxstatic

    # gradle custom params
    property_param = f"""
-Pin_bits
-PVERSION={version}
{param_true_if("enable_cxx_static", cxxstatic_build)}
{param_true_if("enable_16kb_align", args.enable_16kb)}
{param_true_if("disable_publish", args.disable_publish)}
""".replace("\n", " ")

    cmd = f"""./gradlew :AnimaXAndroid:publish {property_param} '-PPUBLISH_INFO={publish_info}'"""
    print(f"execute command: {cmd}")
    result = os.system(cmd)
    if result != 0:
        result = 1
        sys.stderr.write(f"""ERROR: publish AnimaXAndroid failed, params:{property_param}""")
        sys.stderr.flush()
    sys.exit(result)


if __name__ == "__main__":
    main()
