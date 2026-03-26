#!/bin/bash
set -e

# Define color codes
GREEN='\033[0;32m'
NC='\033[0m' # No Color
LIBANIMAX_CONFORMANCE_TEST="animax_conformance_test"

if [[ "$(uname)" == "Darwin" ]]; then

LIBANIMAX_CONFORMANCE_TEST_IN="libanimax_conformance_test.dylib"
LIBGLAPI_IN="libglapi.0.dylib"
LIBOSMESA_IN="libOSMesa.8.dylib"
LIBSKITY_IN="libskity.dylib"

LIBANIMAX_CONFORMANCE_TEST_OUT="$LIBANIMAX_CONFORMANCE_TEST_IN"
LIBGLAPI_OUT="$LIBGLAPI_IN"
LIBOSMESA_OUT="$LIBOSMESA_OUT"
LIBSKITY_OUT="$LIBSKITY_OUT"

elif [[ "$(uname)" == "Linux" ]]; then

LIBANIMAX_CONFORMANCE_TEST_IN="libanimax_conformance_test.so"
LIBGLAPI_IN="libglapi.so.0.0.0"
LIBOSMESA_IN="libOSMesa.so.8.0.0"
LIBSKITY_IN="libskity.so"

LIBANIMAX_CONFORMANCE_TEST_OUT="$LIBANIMAX_CONFORMANCE_TEST_IN"
LIBGLAPI_OUT="libglapi.so"
LIBOSMESA_OUT="libOSMesa.so.8"
LIBOSMESA_OUT2="libOSMesa.so"
LIBSKITY_OUT="$LIBSKITY_OUT"

fi

# Functions for printing messages
print_info() {
  echo "${GREEN}[INFO] $1${NC}"
}

print_info "Compiling ${LIBANIMAX_CONFORMANCE_TEST}..."

# 1. Find the root path of the current git repo
ROOT_PATH=$(git rev-parse --show-toplevel)

ANIMAX_SOURCE_DIR="${ROOT_PATH}/animax"
DIST_DIR=$1

# 2. Change directory to the root path of the repo
cd "$ROOT_PATH" || exit 1

if [ ! -f "$ROOT_PATH/out/Default/build.ninja" ] || ! grep -q "${LIBANIMAX_CONFORMANCE_TEST}" "$ROOT_PATH/out/Default/build.ninja"; then
  # 4. Execute the gn gen command
  ${ROOT_PATH}/buildtools/gn/gn gen out/Default --args="enable_unittests = true"

  # 5. Execute the clang compile database generation script
  ${ROOT_PATH}/tools/clang/scripts/generate_compdb.py -p out/Default > compile_commands.json
fi

# 6. Execute the ninja build command
${ROOT_PATH}/buildtools/ninja/ninja -C out/Default animax_conformance_test

# 7. Copy the binary to the lib directory
# Check if the 'dist' directory exists
if [ ! -d "$DIST_DIR" ]; then
  mkdir "$DIST_DIR"
fi

cp "$ROOT_PATH/out/Default/${LIBANIMAX_CONFORMANCE_TEST_IN}" "$DIST_DIR/${LIBANIMAX_CONFORMANCE_TEST_OUT}"
cp "$ROOT_PATH/out/Default/${LIBGLAPI_IN}" "$DIST_DIR/${LIBGLAPI_OUT}"
cp "$ROOT_PATH/out/Default/${LIBOSMESA_IN}" "$DIST_DIR/${LIBOSMESA_OUT}"
if [ "${LIBOSMESA_OUT2+x}" ]; then
  cp "$ROOT_PATH/out/Default/${LIBOSMESA_IN}" "$DIST_DIR/${LIBOSMESA_OUT2}"
fi
cp "$ROOT_PATH/out/Default/${LIBSKITY_IN}" "$DIST_DIR/${LIBSKITY_OUT}"

print_info "lib${LIBANIMAX_CONFORMANCE_TEST} compiled successfully!"
