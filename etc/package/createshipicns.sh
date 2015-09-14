#!/bin/sh -e

if [ ! -w "$1" ] ; then
	echo "Usage: $0 /path/to/image/to/create/icns/file/from" >&2
	exit 1
fi

basedir=`dirname "$0"`

# Clean out old images
echo "Creating $basedir/icon.iconset directory..."
rm -Rf "$basedir/icon.iconset"
mkdir "$basedir/icon.iconset"

# Resize images
echo "Resizing images..."
convert "$1" -resize 1024x1024 "$basedir/icon.iconset/icon_512x512@2x.png"
convert "$1" -resize 512x512 "$basedir/icon.iconset/icon_512x512.png"
convert "$1" -resize 512x512 "$basedir/icon.iconset/icon_256x256@2x.png"
convert "$1" -resize 256x256 "$basedir/icon.iconset/icon_256x256.png"
convert "$1" -resize 256x256 "$basedir/icon.iconset/icon_128x128@2x.png"
convert "$1" -resize 128x128 "$basedir/icon.iconset/icon_128x128.png"
convert "$1" -resize 64x64 "$basedir/icon.iconset/icon_32x32@2x.png"
convert "$1" -resize 32x32 "$basedir/icon.iconset/icon_32x32.png"
convert "$1" -resize 32x32 "$basedir/icon.iconset/icon_16x16@2x.png"
convert "$1" -resize 16x16 "$basedir/icon.iconset/icon_16x16.png"

# Build icns file
echo "Building icns file"
iconutil --convert icns -o "$basedir/ship.icns" "$basedir/icon.iconset"

#mv "$basedir/icon.icns" ../icon.icns
