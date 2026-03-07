#!/bin/bash
set -e

BASE_DIR="$(cd "$(dirname "$0")" && pwd)"
LOGO_PNG="$BASE_DIR/../logo.png"
ICONSET_DIR="$BASE_DIR/desktop-gallery.iconset"
OUT_ICNS="$BASE_DIR/desktop-gallery.icns"

if [ ! -f "$LOGO_PNG" ]; then
    echo "Error: $LOGO_PNG not found!"
    exit 1
fi

echo "Creating iconset directory..."
mkdir -p "$ICONSET_DIR"

echo "Generating icons with sips..."
sips -z 16 16     "$LOGO_PNG" --out "$ICONSET_DIR/icon_16x16.png"
sips -z 32 32     "$LOGO_PNG" --out "$ICONSET_DIR/icon_16x16@2x.png"
sips -z 32 32     "$LOGO_PNG" --out "$ICONSET_DIR/icon_32x32.png"
sips -z 64 64     "$LOGO_PNG" --out "$ICONSET_DIR/icon_32x32@2x.png"
sips -z 128 128   "$LOGO_PNG" --out "$ICONSET_DIR/icon_128x128.png"
sips -z 256 256   "$LOGO_PNG" --out "$ICONSET_DIR/icon_128x128@2x.png"
sips -z 256 256   "$LOGO_PNG" --out "$ICONSET_DIR/icon_256x256.png"
sips -z 512 512   "$LOGO_PNG" --out "$ICONSET_DIR/icon_256x256@2x.png"
sips -z 512 512   "$LOGO_PNG" --out "$ICONSET_DIR/icon_512x512.png"
sips -z 1024 1024 "$LOGO_PNG" --out "$ICONSET_DIR/icon_512x512@2x.png"

echo "Converting iconset to icns..."
iconutil -c icns "$ICONSET_DIR" -o "$OUT_ICNS"

echo "Cleaning up..."
rm -R "$ICONSET_DIR"

echo "Done! Generated $OUT_ICNS"
