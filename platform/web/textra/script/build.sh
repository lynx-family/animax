#!/bin/bash -e

# Build script for @lynx-js/animax-textra npm package.
#
# Textra's native WebAssembly side module is declared by animax/src/BUILD.gn.
# This script invokes the shared wasm build helper, copies animax_textra.wasm
# into this package, and compiles the TypeScript entrypoint.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ANIMAX_ROOT="$(cd "$SCRIPT_DIR/../../../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/out"
OUT_LIB="$BUILD_DIR/lib"
OUT_LICENSES="$BUILD_DIR/licenses"

BUILD_TYPE="Release"
DO_CLEAN=false

for arg in "$@"; do
  case $arg in
    debug) BUILD_TYPE="Debug" ;;
    clean) DO_CLEAN=true ;;
    *)
      echo "Unknown argument: $arg"
      echo "Usage: $0 [debug] [clean]"
      exit 1
      ;;
  esac
done

if [ "$DO_CLEAN" = true ]; then
  echo "--- Cleaning ---"
  rm -rf "$BUILD_DIR/index.js" "$BUILD_DIR/index.d.ts" \
         "$BUILD_DIR/index.js.in" "$BUILD_DIR/index.d.ts.in" \
         "$BUILD_DIR/wasm" "$OUT_LIB" "$OUT_LICENSES" \
         "$PROJECT_ROOT/index.js" "$PROJECT_ROOT/index.d.ts" "$PROJECT_ROOT/lib"
  echo "Done."
  exit 0
fi

echo "===== AnimaX Textra Build ======"
echo "Mode: $BUILD_TYPE"

python3 "$ANIMAX_ROOT/tools/build_wasm.py" -t "$BUILD_TYPE" --package textra

echo "--- Compiling TypeScript ---"
cd "$PROJECT_ROOT"
npx tsc --project tsconfig.json

if [ ! -f "$OUT_LIB/animax-textra.wasm" ]; then
  echo "ERROR: expected $OUT_LIB/animax-textra.wasm"
  exit 1
fi

echo "--- Staging license notices ---"
mkdir -p "$OUT_LICENSES"
cp "$ANIMAX_ROOT/LICENSE" "$OUT_LICENSES/Apache-2.0.txt"
cp "$ANIMAX_ROOT/third_party/lynx-textra/NOTICE" \
  "$OUT_LICENSES/LynxTextra-NOTICE.txt"
cp "$ANIMAX_ROOT/third_party/harfbuzz/COPYING" \
  "$OUT_LICENSES/HarfBuzz-COPYING.txt"
cp "$ANIMAX_ROOT/third_party/icu/LICENSE" "$OUT_LICENSES/ICU-LICENSE.txt"

echo "===== Build complete ======"
echo "Published package structure:"
ls -la "$BUILD_DIR/"index.* "$OUT_LIB/"*.wasm 2>&1
du -sh "$OUT_LIB/"*.wasm 2>/dev/null || true
