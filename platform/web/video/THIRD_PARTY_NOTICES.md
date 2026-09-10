# Third-Party Notices

`@lynx-js/animax-video` contains code from AnimaX and a WebAssembly binary
linked with FFmpeg.

## AnimaX

The AnimaX source code is licensed under the Apache License 2.0. A copy is
included in `out/licenses/Apache-2.0.txt` in the published package.

## FFmpeg

- Project: [FFmpeg](https://ffmpeg.org/)
- Source: [FFmpeg/FFmpeg](https://github.com/FFmpeg/FFmpeg)
- Release: `n7.0.3`
- Revision: `eaddd1d7140bab19e5a4403d3c0f61fe5f59cb75`
- License configuration: LGPL-2.1-or-later, with GPL, version-3-only, and
  nonfree components disabled

The exact FFmpeg revision is pinned in the repository `DEPS` file. Run
`tools/hab sync .` from the repository root to obtain the corresponding
source. The complete configuration and rebuild steps are in
`platform/web/video/script/build_ffmpeg.sh` and
`platform/web/video/script/build.sh`; they allow a modified FFmpeg build to be
relinked into `animax-video.wasm`.

Copies of FFmpeg's LGPL 2.1 text and license overview are included under
`out/licenses/` in the published package. FFmpeg is provided without warranty.
