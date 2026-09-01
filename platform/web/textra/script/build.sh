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

EMSDK_ENV="$ANIMAX_ROOT/buildtools/emsdk/emsdk_env.sh"
if [ -f "$EMSDK_ENV" ]; then
  # Keep the package script self-contained for local npm builds. Emscripten's
  # em++ launcher uses /usr/bin/env python3, so python3 must also point at a
  # modern interpreter while ninja is running.
  EMSDK_QUIET=1 source "$EMSDK_ENV" >/dev/null
fi

find_modern_python() {
  if [ -n "${EMSDK_PYTHON:-}" ] && [ -x "$EMSDK_PYTHON" ]; then
    echo "$EMSDK_PYTHON"
    return
  fi

  for candidate in python3.12 python3.11 python3.10 python3; do
    if ! command -v "$candidate" >/dev/null 2>&1; then
      continue
    fi
    local python_path
    python_path="$(command -v "$candidate")"
    if "$python_path" - <<'PY'
import sys

sys.exit(0 if sys.version_info >= (3, 10) else 1)
PY
    then
      echo "$python_path"
      return
    fi
  done

  echo "ERROR: Emscripten requires Python 3.10 or newer." >&2
  exit 1
}

PYTHON_BIN="$(find_modern_python)"
export EMSDK_PYTHON="$PYTHON_BIN"
export PATH="$(dirname "$PYTHON_BIN"):$PATH"

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

"$PYTHON_BIN" "$ANIMAX_ROOT/tools/build_wasm.py" -t "$BUILD_TYPE" --package textra

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
cp "$PROJECT_ROOT/THIRD_PARTY_NOTICES.md" \
  "$OUT_LICENSES/THIRD_PARTY_NOTICES.md"

echo "===== Build complete ======"
echo "Published package structure:"
ls -la "$BUILD_DIR/"index.* "$OUT_LIB/"*.wasm 2>&1
du -sh "$OUT_LIB/"*.wasm 2>/dev/null || true
