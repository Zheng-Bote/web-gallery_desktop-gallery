#!/bin/bash
set -e

BASE_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$BASE_DIR/build"

QT_DIR="/Users/zb_bamboo/Qt/6.10.2/macos"

echo "Building Desktop-Gallery..."
export PATH="$QT_DIR/bin:$PATH"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -DCMAKE_PREFIX_PATH="$QT_DIR" \
      -DQt6_DIR="$QT_DIR/lib/cmake/Qt6" \
      -DCMAKE_BUILD_TYPE=Release ..
make -j$(sysctl -n hw.ncpu)

echo "Running macdeployqt..."
MACDEPLOYQT="$QT_DIR/bin/macdeployqt"

if [ ! -x "$MACDEPLOYQT" ]; then
    echo "Error: macdeployqt not found at $MACDEPLOYQT"
    exit 1
fi

"$MACDEPLOYQT" desktop-gallery.app -always-overwrite 

echo "Ad-hoc signing the App Bundle to fix ARM64 killed: 9 crash..."
codesign --force --deep --sign - desktop-gallery.app

echo "Packaging with CPack..."
cpack -G DragNDrop

echo "Done! DMG should be located in $BUILD_DIR/"
