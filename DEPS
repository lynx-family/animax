import os
import platform

system = platform.system().lower()
machine = platform.machine().lower()
machine = "x86_64" if machine == "amd64" else machine

python_path = "python3"
if system == "windows":
    python_path = "python"

deps = {
    # Build environment setup
    'platform/android/gradle/wrapper/gradle-6.7.1-all.zip': {
        "type": "http",
        "url": "https://services.gradle.org/distributions/gradle-6.7.1-all.zip",
        "decompress": False,
    },
    'example/android/gradle/wrapper/gradle-6.7.1-all.zip': {
        "type": "http",
        "url": "https://services.gradle.org/distributions/gradle-6.7.1-all.zip",
        "decompress": False,
    },
    'buildtools/ninja': {
        "type": "http",
        "url": {
            "linux": "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip",
            "darwin": "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-mac.zip",
            "windows": "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-win.zip"
        }.get(system, None),
        "sha256": {
            "linux": "b901ba96e486dce377f9a070ed4ef3f79deb45f4ffe2938f8e7ddc69cfb3df77",
            "darwin": "482ecb23c59ae3d4f158029112de172dd96bb0e97549c4b1ca32d8fad11f873e",
            "windows": "524b344a1a9a55005eaf868d991e090ab8ce07fa109f1820d40e74642e289abc"
        }.get(system, None),
        "ignore_in_git": True,
        "condition": system in ['linux', 'darwin', 'windows']
    },
    "buildtools/node": {
        "type": "http",
        "url": {
            "linux-x86_64": "https://nodejs.org/dist/v18.19.1/node-v18.19.1-linux-x64.tar.gz",
            "linux-arm64": "https://nodejs.org/dist/v18.19.1/node-v18.19.1-linux-arm64.tar.gz",
            "darwin-x86_64": "https://nodejs.org/dist/v18.19.1/node-v18.19.1-darwin-x64.tar.gz",
            "darwin-arm64": "https://nodejs.org/dist/v18.19.1/node-v18.19.1-darwin-arm64.tar.gz",
            "windows-x86_64": "https://nodejs.org/dist/v18.19.1/node-v18.19.1-win-x64.zip"
        }.get(f'{system}-{machine}', None),
        "sha256": {
            "linux-x86_64": "724802c45237477dbe5777923743e6c77906830cae03a82b5653ebd75b301dda",
            "linux-arm64": "2913e8544d95c8be9e6034c539ec0584014532166a088bf742629756c3ec42e2",
            "darwin-x86_64": "ab67c52c0d215d6890197c951e1bd479b6140ab630212b96867395e21d813016",
            "darwin-arm64": "0c7249318868877032ed21cc0ed450015ee44b31b9b281955521cd3fc39fbfa3",
            "windows-x86_64": "ff08f8fe253fba9274992d7052e9d9a70141342d7b36ddbd6e84cbe823e312c6"
        }.get(f'{system}-{machine}', None),
        "ignore_in_git": True,
        "condition": system in ['linux', 'darwin', 'windows']
    },
    'buildtools/gn': {
        "type": "http",
        "url": f"https://github.com/lynx-family/buildtools/releases/download/gn-cc28efe6/buildtools-gn-{system}-{machine}.tar.gz",
        "ignore_in_git": True,
        "condition": system in ['linux', 'darwin', 'windows']
    },
    'buildtools/llvm': {
        "type": "http",
        'url': f"https://github.com/lynx-family/buildtools/releases/download/llvm-020d2fb7/buildtools-llvm-{system}-{machine}.tar.gz",
        "ignore_in_git": True,
        "decompress": True,
        "condition": system in ['linux', 'darwin'],
    },
    'buildtools/cmake': {
        "type": "http",
        "url": {
            "linux": f"https://cmake.org/files/v3.18/cmake-3.18.1-Linux-x86_64.tar.gz",
            "darwin": f"https://dl.google.com/android/repository/ba34c321f92f6e6fd696c8354c262c122f56abf8.cmake-3.18.1-darwin.zip",
            "windows": f"https://cmake.org/files/v3.18/cmake-3.18.1-win64-x64.zip"
        }.get(system, None),
        "sha256": {
            "linux": "537de8ad3a7fb4ec9b8517870db255802ad211aec00002c651e178848f7a769e",
            "darwin": "b15d6d7ab5615a48bb14962f5a931be6cd9a0c187f4bd6be404bdd46a7bef60b",
            "windows": "2c6c06da43c1088fc3a673e4440c8ebb1531bb6511134892c0589aa0b94f11ad"
        }.get(system, None),
        "ignore_in_git": True,
        "condition": system in ['linux', 'darwin', 'windows']
    },
    "build": {
        "type": "git",
        "url": "https://github.com/lynx-family/buildroot.git",
        "commit": "2b6a631306054a2ff884b978ead2eafeaf0b328f",
        "ignore_in_git": True,
        "condition": system in ['linux', 'darwin', 'windows']
    },
    "build/linux/debian_sid_amd64-sysroot": {
        "type": "http",
        "url": "https://commondatastorage.googleapis.com/chrome-linux-sysroot/toolchain/79a7783607a69b6f439add567eb6fcb48877085c/debian_sid_amd64_sysroot.tar.xz",
        "ignore_in_git": True,
        "condition": machine == "x86_64" and system == "linux",
        "require": ["build"]
    },
    'third_party/gyp': {
        "type": "git",
        "url": "https://chromium.googlesource.com/external/gyp",
        "commit": "9d09418933ea2f75cc416e5ce38d15f62acd5c9a",
        "ignore_in_git": True,
        "condition": system in ['linux', 'darwin', 'windows'],
    },
    'third_party/skity': {
        "type": "solution",
        "url": "https://github.com/lynx-family/skity.git",
        "commit": "3fb6bb60a0e94e6c8a01630f7690492ff59aa666",
        "deps_file": "hab/DEPS",
        "ignore_in_git": True,
    },
    # lynx-textra DEPS BEGIN
    'third_party/lynx-textra': {
        "type": "solution",
        "url": "https://github.com/lynx-family/lynx-textra.git",
        "commit": "9d26418825ad2c7d6b8064fceef86b0e65f433ca",
        "deps_file": "DEPS",
        'patches': [
            os.path.join(root_dir, 'patches', 'lynx-textra', '*.patch')
        ],
        "ignore_in_git": True,
    },
    'third_party/harfbuzz': {
        'type': 'git',
        'url': 'https://github.com/harfbuzz/harfbuzz',
        'commit': 'a070f9ebbe88dc71b248af9731dd49ec93f4e6e6',
        'patches': [
            os.path.join(root_dir, 'patches', 'harfbuzz', '*.patch')
        ],
        "ignore_in_git": True,
    },
    'third_party/icu': {
        "type": "git",
        "url": "https://chromium.googlesource.com/chromium/deps/icu",
        "commit": "f90543d272e2e2edc5b3cdf8ead0b5b3eebceef5",
        "ignore_in_git": True,
    },
    # lynx-textra DEPS END
    # lynx-base DEPS BEGIN
    'third_party/lynx': {
        "type": "git",
        "url": "https://github.com/lynx-family/lynx.git",
        "commit": "7edf2700bed58556c2bfcaeaff99231e0c3013db",
        "paths": ["third_party/rapidjson", "base", "platform/android/service_api", "platform/darwin/ios/lynx_service_api"],
        "patches": [
            os.path.join(root_dir, 'patches', 'lynx', '*.patch')
        ],
        "ignore_in_git": True,
    },
    'third_party/modp_b64': {
        'type': 'git',
        'url': 'https://chromium.googlesource.com/chromium/src/third_party/modp_b64',
        'commit': '50685101d51ef9aabbd60c94f52d9e026d39c509',
        "patches": os.path.join(root_dir, 'patches', 'modp_b64', '*.patch'),
        "ignore_in_git": True,
    },
    'third_party/perfetto': {
        'type': 'git',
        'url': 'https://github.com/lynx-family/lynx-trace.git',
        'commit': 'ea35ecb68fdacd889df642fc2eb578c5c58e1c7a',
        "patches": [
                os.path.join(root_dir, 'patches', 'perfetto', '*.patch')
            ],
        "ignore_in_git": True,
        "paths": [ "sdk" ],
    },
    "third_party/xhook": {
        'type': 'git',
        'url': 'https://github.com/iqiyi/xHook.git',
        'commit': 'e59285034feadfdd4ba9b65e1eea1d381da83ed3',
        "patches": os.path.join(root_dir, 'patches', 'xhook', '*.patch'),
        "ignore_in_git": True,
    },
    'third_party/zlib': {
        'type': 'git',
        'url': 'https://chromium.googlesource.com/chromium/src/third_party/zlib',
        'commit': 'f5fd0ad2663e239a31184ad4c9919991dda16f46',
        "patches": os.path.join(root_dir, 'patches', 'zlib', '*.patch'),
        "ignore_in_git": True,
    },
    # lynx-base DEPS END
}
