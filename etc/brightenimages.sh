#!/bin/sh

set -e

function print_usage_and_exit() {
	echo "Usage: '$0' <source folder>" "<destination folder" >&2
	exit 1
}

if [ $# -lt 2 -o ! -d "$1" ] ; then
	print_usage_and_exit
fi

if [ ! -d "$2" ] ; then
	mkdir -p "$2" || exit 1
fi

imagesfiles=`cd "$1" && find . -type f \( -iname "*.png" -o -iname "*.bmp" -o -iname "*.jpg" \)`
if [ -z "$imagesfiles" ] ; then
	echo "Error: no image files found in source folder" >&2
	exit 1
fi

txtfiles=`cd "$1" && find . -type f -iname "*.txt"`
if [ ! -z "$txtfiles" ] ; then
	echo "$txtfiles" | while read i ; do
		mkdir -p "$2/"`dirname "$i"`
		cp "$1/$i" "$2/$i"
	done
fi

echo "$imagesfiles" | while read i ; do
	mkdir -p "$2/"`dirname "$i"`
	convert "$1/$i" -modulate 300 "$2/$i"
done

exit 0
