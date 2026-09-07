#!/bin/bash -e

# Build FFmpeg as WebAssembly static libraries.
#
# Uses the FFmpeg revision pinned in the repository DEPS file and compiles it
# with a minimal LGPL-compatible configuration
# for H.264/H.265 video decoding:
#   - Decoders: h264, hevc
#   - Demuxers: mov, matroska
#   - Protocols: data (for custom AVIO)
#   - Libraries: avcodec, avformat, avutil only
#
# Usage:
#   ./build_ffmpeg.sh                    # Build the pinned source
#   ./build_ffmpeg.sh --clean            # Clean output and rebuild from scratch

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ANIMAX_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"
FFMPEG_SRC="$ANIMAX_ROOT/third_party/ffmpeg"
FFMPEG_OUT="$PROJECT_ROOT/out/ffmpeg"
FFMPEG_BUILD="$FFMPEG_OUT/build"
FFMPEG_INSTALL="$FFMPEG_OUT/wasm"

DO_CLEAN=false

for arg in "$@"; do
  case $arg in
    --clean)      DO_CLEAN=true ;;
    *)
      echo "Unknown argument: $arg"
      echo "Usage: $0 [--clean]"
      exit 1
      ;;
  esac
done

if [ ! -x "$FFMPEG_SRC/configure" ]; then
  echo "FFmpeg source is missing at $FFMPEG_SRC" >&2
  echo "Run 'tools/hab sync .' from $ANIMAX_ROOT." >&2
  exit 1
fi

FFMPEG_REVISION="$(git -C "$FFMPEG_SRC" rev-parse HEAD)"
FFMPEG_EXPECTED_REVISION="$(
  python3 "$SCRIPT_DIR/read_ffmpeg_revision.py" "$ANIMAX_ROOT/DEPS"
)"
if [ "$FFMPEG_REVISION" != "$FFMPEG_EXPECTED_REVISION" ]; then
  echo "Unexpected FFmpeg revision: $FFMPEG_REVISION" >&2
  echo "Expected from DEPS: $FFMPEG_EXPECTED_REVISION" >&2
  echo "Run 'tools/hab sync .' to restore DEPS." >&2
  exit 1
fi

# --- Build ---
echo ""
echo "===== Building FFmpeg for WebAssembly ======"
echo "Source: $FFMPEG_SRC"
echo "Output: $FFMPEG_OUT"

if [ "$DO_CLEAN" = true ]; then
  echo "--- Cleaning previous build ---"
  rm -rf "$FFMPEG_OUT"
fi

mkdir -p "$FFMPEG_BUILD" "$FFMPEG_INSTALL"

cd "$FFMPEG_BUILD"

echo "--- Configuring FFmpeg ---"
emconfigure "$FFMPEG_SRC/configure" \
    --cc="emcc" \
    --cxx="em++" \
    --ar="emar" \
    --ranlib="emranlib" \
    --target-os=none \
    --arch=x86_32 \
    --cpu=generic \
    --enable-cross-compile \
    --disable-asm \
    --disable-inline-asm \
    --disable-autodetect \
    --disable-programs \
    --disable-doc \
    --disable-all \
    --disable-gpl \
    --disable-version3 \
    --disable-nonfree \
    --disable-network \
    --enable-small \
    --enable-static \
    --enable-avcodec \
    --enable-avformat \
    --enable-avutil \
    --enable-decoder=h264 \
    --enable-decoder=hevc \
    --enable-demuxer=mov \
    --enable-demuxer=matroska \
    --enable-parser=h264 \
    --enable-parser=hevc \
    --enable-protocol=data \
    --extra-cflags="-fPIC -Os -ffunction-sections -fdata-sections" \
    --extra-cxxflags="-fPIC -Os -ffunction-sections -fdata-sections" \
    --prefix="$FFMPEG_INSTALL"

echo "--- Compiling FFmpeg ---"
if command -v nproc >/dev/null 2>&1; then
  BUILD_JOBS="$(nproc)"
elif command -v sysctl >/dev/null 2>&1; then
  BUILD_JOBS="$(sysctl -n hw.ncpu)"
else
  BUILD_JOBS=4
fi
make -j"$BUILD_JOBS"

echo "--- Installing ---"
make install

printf '%s\n' "$FFMPEG_REVISION" > "$FFMPEG_INSTALL/.build_revision"

echo "===== Done: FFmpeg static libraries ====="
echo "Output: $FFMPEG_INSTALL/lib/"
ls -lh "$FFMPEG_INSTALL/lib/"*.a 2>/dev/null || echo "(no .a files found)"
