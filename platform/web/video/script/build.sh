#!/bin/bash -e

# Build script for @lynx-js/animax-video npm package.
#
# Orchestrates FFmpeg compilation, C++ to WASM linking, and
# TypeScript compilation for the publishable npm package.
#
# Usage:
#   ./script/build.sh                  # Release build (incremental)
#   ./script/build.sh debug            # Debug build (no optimization)
#   ./script/build.sh --force-ffmpeg    # Force recompile FFmpeg from source
#   ./script/build.sh clean            # Remove all build output

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ANIMAX_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/out"
FFMPEG_LIB="$PROJECT_ROOT/out/ffmpeg/wasm"
FFMPEG_SRC="$ANIMAX_ROOT/third_party/ffmpeg"
NATIVE_DIR="$PROJECT_ROOT/native"
OUT_LIB="$BUILD_DIR/lib"
OUT_LICENSES="$BUILD_DIR/licenses"

EMSDK_ENV="$ANIMAX_ROOT/buildtools/emsdk/emsdk_env.sh"
if [ -f "$EMSDK_ENV" ]; then
  EMSDK_QUIET=1 source "$EMSDK_ENV" >/dev/null
fi

# Parse arguments
BUILD_TYPE="Release"
FORCE_FFMPEG=false
DO_CLEAN=false

for arg in "$@"; do
  case $arg in
    debug)         BUILD_TYPE="Debug" ;;
    --force-ffmpeg) FORCE_FFMPEG=true ;;
    clean)          DO_CLEAN=true ;;
    *)
      echo "Unknown argument: $arg"
      echo "Usage: $0 [debug] [--force-ffmpeg] [clean]"
      exit 1
      ;;
  esac
done

# --- Clean ---
if [ "$DO_CLEAN" = true ]; then
  echo "--- Cleaning ---"
  rm -rf "$BUILD_DIR/index.js" "$BUILD_DIR/index.d.ts" \
         "$BUILD_DIR/index.js.in" "$BUILD_DIR/index.d.ts.in" \
         "$BUILD_DIR/wasm" "$OUT_LIB" \
         "$OUT_LICENSES" \
         "$PROJECT_ROOT/index.js" "$PROJECT_ROOT/index.d.ts" "$PROJECT_ROOT/lib"
  echo "Done."
  exit 0
fi

echo "===== AnimaX Video Build ======"
echo "Mode: $BUILD_TYPE"

# --- Prepare directories ---
mkdir -p "$OUT_LIB"

# --- FFmpeg (compile if missing or forced) ---
FFMPEG_REVISION_FILE="$FFMPEG_LIB/.build_revision"
FFMPEG_EXPECTED_REVISION="$(
  python3 "$SCRIPT_DIR/read_ffmpeg_revision.py" "$ANIMAX_ROOT/DEPS"
)"
FFMPEG_SOURCE_REVISION="$(git -C "$FFMPEG_SRC" rev-parse HEAD 2>/dev/null || true)"

build_ffmpeg() {
  echo "--- Building FFmpeg ---"
  "$SCRIPT_DIR/build_ffmpeg.sh" "$@"
}

if [ "$FORCE_FFMPEG" = true ]; then
  build_ffmpeg --clean
elif [ ! -f "$FFMPEG_REVISION_FILE" ] || \
     [ -z "$FFMPEG_SOURCE_REVISION" ] || \
     [ "$FFMPEG_SOURCE_REVISION" != "$FFMPEG_EXPECTED_REVISION" ] || \
     [ "$(cat "$FFMPEG_REVISION_FILE")" != "$FFMPEG_EXPECTED_REVISION" ] || \
     [ -z "$(ls -A "$FFMPEG_LIB/lib"/*.a 2>/dev/null)" ]; then
  echo "--- FFmpeg not found, building ---"
  build_ffmpeg
else
  echo "--- FFmpeg cached, skipping (use --force-ffmpeg to rebuild) ---"
fi

# --- Link C++ to WASM ---
echo "--- Linking WebAssembly ($BUILD_TYPE) ---"

if [ "$BUILD_TYPE" = "Debug" ]; then
  EMCC_FLAGS="-O0 -Wno-limited-postlink-optimizations"
else
  EMCC_FLAGS="-Oz"
fi

cd "$PROJECT_ROOT"
emcc $EMCC_FLAGS -std=c++17 \
  -fPIC \
  -fno-exceptions \
  -fno-rtti \
  -I"$NATIVE_DIR" \
  -I"$FFMPEG_LIB/include" \
  "$FFMPEG_LIB/lib/libavcodec.a" \
  "$FFMPEG_LIB/lib/libavformat.a" \
  "$FFMPEG_LIB/lib/libavutil.a" \
  "$NATIVE_DIR/ffmpeg_decoder_web.cc" \
  "$NATIVE_DIR/video_c_api.cc" \
  -s WASM=1 \
  -s SIDE_MODULE=2 \
  -o "$OUT_LIB/animax-video.wasm"

# --- Stage license and corresponding-source information ---
echo "--- Staging license notices ---"
mkdir -p "$OUT_LICENSES"
cp "$ANIMAX_ROOT/LICENSE" "$OUT_LICENSES/Apache-2.0.txt"
cp "$FFMPEG_SRC/COPYING.LGPLv2.1" "$OUT_LICENSES/FFmpeg-LGPL-2.1.txt"
cp "$FFMPEG_SRC/LICENSE.md" "$OUT_LICENSES/FFmpeg-LICENSE.md"

# --- Compile TypeScript source ---
echo "--- Compiling TypeScript ---"
cd "$PROJECT_ROOT"
npx tsc --project tsconfig.json

echo "===== Build complete ======"
echo "Published package structure:"
ls -la "$BUILD_DIR/"index.* "$OUT_LIB/"*.wasm 2>&1
du -sh "$OUT_LIB/"*.wasm 2>/dev/null || true
