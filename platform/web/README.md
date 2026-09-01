# AnimaX Web Platform

This directory contains three npm packages for the AnimaX Web platform:

## Directory Structure

```
web/
├── core/                          @lynx-js/animax
│   ├── src/                       TypeScript source (view, gpu, resource, etc.)
│   ├── package.json               npm package config
│   ├── rspack.config.js           Bundler config
│   └── README.md                  Package API documentation
│
├── video/                         @lynx-js/animax-video
│   ├── src/index.ts               TypeScript source (exports AnimaXVideoModuleUrl)
│   ├── script/                    Build scripts
│   ├── package.json               npm package config
│   └── README.md                  Package documentation
│
└── textra/                        @lynx-js/animax-textra
    ├── src/index.ts               TypeScript source (exports AnimaXTextraModuleUrl)
    ├── script/                    Build scripts
    ├── package.json               npm package config
    └── README.md                  Package documentation
```

## Packages

### `@lynx-js/animax` (`core/`)

The core animation rendering library for Web. Provides the `<animax-view>` custom element with WebGL/WebGPU backends, supporting Lottie/AnimaX JSON animations.

- **Entry**: `out/index.js`
- **Dependencies**: None (runtime)
- **Optional runtime modules**: `@lynx-js/animax-video` for video layers and `@lynx-js/animax-textra` for Textra text layout. Keep companion package versions aligned with core.
- **See**: [core/README.md](./core/README.md) for full API reference

### `@lynx-js/animax-video` (`video/`)

Companion video decoding module. Compiles FFmpeg (H.264/H.265) to WebAssembly for video layer support in AnimaX animations.

- **Entry**: `out/index.js` (exports `AnimaXVideoModuleUrl`)
- **Use with**: `@lynx-js/animax` at the same version
- **See**: [video/README.md](./video/README.md) for build details and architecture

### `@lynx-js/animax-textra` (`textra/`)

Companion Textra text layout module. Provides a WebAssembly side module used by `AnimaXViewElement.loadTextraModule`.

- **Entry**: `out/index.js` (exports `AnimaXTextraModuleUrl`)
- **Use with**: `@lynx-js/animax` at the same version
- **See**: [textra/README.md](./textra/README.md) for usage and build details

## Local Development

This repository contains all three Web packages.

### Setup

```bash
tools/hab sync .
source tools/envsetup.sh

npm install --prefix platform/web/core
npm install --prefix platform/web/video
npm install --prefix platform/web/textra
```

### Build & Run

1. **Build core module**:

```bash
cd platform/web/core
npm run build
```

2. **Build video module** (optional, only if using video layers):

```bash
cd platform/web/video
npm run build
```

3. **Build Textra module** (optional, only if using Textra text layout):

```bash
cd platform/web/textra
npm run build
```

The video package uses the upstream FFmpeg revision pinned by `DEPS`; it does
not download dependencies from its npm build script. Its published Wasm bundle
includes FFmpeg under LGPL-2.1-or-later. See
[`video/THIRD_PARTY_NOTICES.md`](./video/THIRD_PARTY_NOTICES.md).

For a package integrity check, run `npm pack --dry-run` in its directory after
the build.

## License

The core and Textra package code is Apache-2.0. The video package also ships an
FFmpeg-derived Wasm component under LGPL-2.1-or-later; see
[`video/THIRD_PARTY_NOTICES.md`](./video/THIRD_PARTY_NOTICES.md).
