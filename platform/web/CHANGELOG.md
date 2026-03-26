# Changelog

All notable changes to this project will be documented in this file.

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
