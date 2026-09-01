# @lynx-js/animax-video

FFmpeg-based H.264/H.265 video decoder compiled to **WebAssembly (WASM)** for the AnimaX animation framework on the Web platform.

## Features

- **H.264 (AVC)** and **H.265 (HEVC)** video decoding
- In-memory data input via custom AVIO callbacks (no file I/O)
- MP4 / MKV container support
- YUV420P / YUV422P / YUV444P pixel format output
- Minimal WASM binary size (~1.9 MB) through aggressive FFmpeg configuration

## Installation

Install this package only when your AnimaX animation contains video layers. Keep its version aligned with `@lynx-js/animax`.

```bash
npm install @lynx-js/animax@latest \
  @lynx-js/animax-video@latest
```

## Usage

`@lynx-js/animax-video` only provides the wasm URL. Load it through the core package before mounting `<animax-view>` instances that need video layer playback.

```typescript
import { AnimaXViewElement } from '@lynx-js/animax';
import { AnimaXVideoModuleUrl } from '@lynx-js/animax-video';

const videoLoaded = await AnimaXViewElement.loadVideoModule(
  AnimaXVideoModuleUrl
);
if (!videoLoaded) {
  throw new Error('Failed to load AnimaX video module');
}
```

On the Web platform, video layers do not support ZIP-compressed video resources. Use JSON animations with accessible video resource URLs.

## Prerequisites

- [Emscripten SDK](https://emscripten.org/) (tested with 5.x)
- Node.js ≥ 18
- CMake (via Emscripten)

## Quick Start

```bash
# 1. Install dependencies
npm install

# 2. Sync the pinned FFmpeg source from the repository root
cd ../../..
tools/hab sync .
cd platform/web/video

# 3. Build FFmpeg + link WASM (Release mode, recommended)
./script/build_ffmpeg.sh                    # Build the revision pinned by DEPS
./script/build_ffmpeg.sh --force            # Clean and rebuild from scratch

# 4. Build full package
./script/build.sh

# Or build in Debug mode
./script/build.sh debug
```

## Build Commands

### FFmpeg

```bash
./script/build_ffmpeg.sh                    # Build the DEPS-pinned source
./script/build_ffmpeg.sh --clean            # Clean output and rebuild
./script/build_ffmpeg.sh --force            # Alias for --clean
```

### NPM scripts (from package.json)

| Script | Command | Description |
|--------|--------|-------------|
| `build` | `npm run clean && ./script/build.sh` | Full clean + Release build |
| `build:debug` | `npm run clean && ./script/build.sh debug` | Full clean + Debug build |
| `build:ffmpeg` | `./script/build_ffmpeg.sh` | Build the pinned FFmpeg source |
| `build:ffmpeg:force` | `./script/build_ffmpeg.sh --force` | Clean + rebuild FFmpeg |
| `clean` | Remove build output | Clean all artifacts |

## Output

After building, the publishable package contents are in `out/`:

```
@lynx-js/animax-video/
└── out/
    ├── index.js                  # JS entry (exports AnimaXVideoModuleUrl)
    ├── index.d.ts                # TypeScript declarations
    └── lib/
        └── animax-video.wasm     # WebAssembly decoder module (~1.9 MB)
```

## Architecture

```
JavaScript / TypeScript
        │
        │  import { AnimaXVideoModuleUrl } from '@lynx-js/animax-video'
        │  loadVideoModule(url) → fetch .wasm → WebAssembly.instantiate
        ▼
┌─────────────────────────────────┐
│   out/lib/animax-video.wasm      │  ← emcc -Oz -s SIDE_MODULE=1
│                                   │
│   namespace video_wasm {          │  C++ → WASM
│     CreateVideoDecoder()         │
│     DecodeFrame()               │
│     PrepareFrameData()           │
│     GetFrameData()              │
│   }                             │
│                                   │
│   Static libs (linked in):      │
│   ├── libavcodec.a  (h264+hevc) │
│   ├── libavformat.a (mov+mkv)   │
│   └── libavutil.a             │
└─────────────────────────────────┘
```

## API (C ABI exported by WASM)

| Function | Description |
|----------|-------------|
| `animax_video_decoder_alloc(data, size)` | Create decoder from raw bytes |
| `animax_video_decoder_free(decoder)` | Destroy decoder |
| `animax_video_prepare_frame_data(decoder, ...)` | Extract all frame packets |
| `animax_video_frame_alloc(decoder, data, size)` | Decode single frame |
| `animax_video_get_frame_data(frame, ...)` | Read YUV plane data |
| `animax_video_frame_free(frame)` | Release decoded frame |

## FFmpeg Configuration

The FFmpeg static libraries are built with minimal configuration to minimize WASM size:

| Category | Enabled |
|----------|---------|
| Decoders | h264, hevc |
| Demuxers | mov, matroska |
| Parsers | h264, hevc |
| Protocols | data |
| Libraries | avcodec, avformat, avutil |

Everything else is disabled: encoders, muxers, filters, devices, swresample, swscale, network, hwaccels, pthreads, etc.

## License

The AnimaX package code is Apache-2.0. The distributed WebAssembly module is
linked with FFmpeg configured under LGPL-2.1-or-later. The npm package includes
the applicable license texts and exact corresponding-source information; see
[`THIRD_PARTY_NOTICES.md`](./THIRD_PARTY_NOTICES.md).
