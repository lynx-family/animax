# AnimaX Web

AnimaX Web is distributed as three npm packages:

- `@lynx-js/animax` provides the `<animax-view>` Web component and its
  WebAssembly runtime.
- `@lynx-js/animax-video` adds H.264 and H.265 video decoding.
- `@lynx-js/animax-textra` adds Textra text layout.

The video and Textra packages are optional. Keep all installed AnimaX packages
at the same version.

## Install

```bash
npm install @lynx-js/animax
```

Install the companion packages only when the animation needs them:

```bash
npm install @lynx-js/animax-video @lynx-js/animax-textra
```

See each package README for its minimal usage example.

## Build

From the repository root:

```bash
tools/hab sync .
source tools/envsetup.sh

cd platform/web/core
npm install
npm run build
```

Use the same commands in `platform/web/video` or `platform/web/textra` to build
an optional package.

## License

The core and Textra packages are Apache-2.0. The video package contains an
FFmpeg-derived WebAssembly module under LGPL-2.1-or-later; see
[`video/THIRD_PARTY_NOTICES.md`](./video/THIRD_PARTY_NOTICES.md).
