# Changelog

All notable changes to this project will be documented in this file.

## [1.0.16] - 2026-08-10

### Added
- Added opt-in WebGL frame capture for retaining a rendered frame while a
  canvas is temporarily unavailable.

### Fixed
- Recreated Web players after canvas lifecycle and WebGL context changes,
  preventing blank animation views after remounting or resizing.

## [1.0.15] - 2026-08-05

### Changed
- Added reproducible packaging for prebuilt WebAssembly npm artifacts.

## [1.0.14] - 2026-06-24

### Fixed
- Fixed WebGL rendering when multiple `<animax-view>` instances play at the
  same time. Each WebGL surface now makes its own context current before
  drawing, flushing, resizing, and destroying, preventing later instances from
  stealing the current context and leaving other canvases blank.

## [1.0.13] - 2026-06-24

### Fixed
- Moved WebGL CPU image resource premultiplication out of JavaScript and let Skity convert unpremultiplied pixel data before texture upload.

## [1.0.12] - 2026-06-17

### Fixed
- Fixed web image resource alpha handling by decoding and uploading image resources as premultiplied alpha, aligning the native bitmap alpha type with both the WebGL CPU path and WebGPU texture path.
- Reused the active WebGPU context for resource loading and canvas resize so WebGPU image textures are created against the same rendering context.

## [1.0.11] - 2026-05-08

### Added
- Added Web multi-font registration APIs: `configureFonts`, `loadFonts`, and `loadFont`.
- Added optional Textra wasm module loading via `loadTextraModule`.

### Changed
- Simplified Web font configuration around explicit `family` + `url` descriptors.
- Removed the legacy default-font shortcut. Use `configureFonts`, `loadFonts`, or `loadFont` instead.
- Documented that video and Textra text layout support require `@lynx-js/animax-video` and `@lynx-js/animax-textra` at the same version as `@lynx-js/animax`.

## [1.0.9] - 2026-04-08

### Added
- **Video Layer Support**: Added support for H.264 (AVC) and H.265 (HEVC) video layers in AnimaX animations. Video decoding is powered by FFmpeg compiled to WebAssembly via the companion `@lynx-js/animax-video` package.
  - New static method `AnimaXViewElement.loadVideoModule(url)` loads the WASM decoder module. Must be called once before initializing any `<animax-view>` element that contains video layers.
  - Install the video decoder: `npm install @lynx-js/animax-video`
  - Usage:
    ```typescript
    import { AnimaXViewElement } from '@lynx-js/animax';
    import { AnimaXVideoModuleUrl } from '@lynx-js/animax-video';

    await AnimaXViewElement.loadVideoModule(AnimaXVideoModuleUrl);
    ```

## [1.0.8] - 2026-03-02

### Fixed
- **WebGPU Backend Texture Support**: Improved WebGPU texture creation. WebGPU backend now supports creating textures from downloaded image data in JS, passing the `textureId` to C++, and creating a `HardwareImage` directly from that id for rendering. WebGL backend still uses the previous path: render into an offscreen `Canvas` to produce a `Bitmap`, then pass bitmap data to C++ to create `BitmapImage`.
- **Batch One-Shot VSync Requests**: Optimized WebGPU batching to ensure all render tasks within a VSync window are coalesced, reducing context switches and draw calls. Introduced a singleton `VSyncDispatcher` to merge multiple `RequestVSync()` calls into a single RAF request per frame, execute pending one-shot callbacks on the RAF tick, skip cancelled tokens, and re-arm RAF only when new work is enqueued.
- **Optimize ObjectPosition**: Added `top-left`, `top-right`, `bottom-left`, `bottom-right` enum values to represent object positions on screen.

### Docs
- **Documentation Improvements**: Updated docs to keep `<animax-view>` properties and methods consistent with the implementation.

## [1.0.7] - 2026-01-14

### Changed
- **WebGL Context**: Moved WebGL context creation from C++ to JavaScript using `Module.GL.registerContext`. This enables proper rendering when `<animax-view>` is used inside Shadow DOM.

### Fixed
- **Shadow DOM Support**: Fixed WebGL context creation failure when the canvas element is inside a Shadow DOM (CSS selector couldn't find the element).

## [1.0.6] - 2026-01-11

### Fixed
- **Bindings**: Fixed "unbound types" errors for `setSrcPolyfill` (Map) and `onShow/onHide` (VisibilityState) by implementing manual type conversion in WASM bindings.

## [1.0.5] - 2026-01-08

### Fixed
- **Wasm Build**: Fixed wasm locateFile build error.
- **Consumer Configuration**: Removed the need for complex manual copying of WASM files in consumer projects. Consumers only need to configure their bundler to treat `.wasm` files as resources (e.g., `type: 'asset/resource'` in Rsbuild).

## [1.0.4] - 2026-01-04

### Fixed
- **TypeScript Declaration Files**: Fixed the issue where TypeScript could not find module declaration files, causing implicit `any` type warnings in consumer projects.
- **Package Configuration**: Updated `package.json` to correctly point to generated type declaration files at `out/src/index.d.ts` instead of non-existent `out/index.d.ts`.

## [1.0.3] - 2026-01-02

### Fixed
- **Zero-Config WASM Loading**: Optimized the WASM loading mechanism. The library now exports the WASM file dependency in a way that allows consumer bundlers (like Rsbuild, Webpack) to automatically process it.
- **Consumer Configuration**: Removed the need for complex manual copying of WASM files in consumer projects. Consumers only need to configure their bundler to treat `.wasm` files as resources (e.g., `type: 'asset/resource'` in Rsbuild).

### Changed
- **Build System**: Migrated to **Rspack** as the build scaffold for faster and more efficient builds.
- **Build Output**: The library's build output is now more streamlined, avoiding redundant file copies.
- **Documentation**: Updated `README.md` with clear instructions for React and Rsbuild usage.

## [1.0.2] - 2025-12-26

### Added
- **Project Initialization**: Initial release of the AnimaX web platform library.
