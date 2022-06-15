#!/bin/sh

set -e

function print_usage_and_exit() {
	echo "Usage: '$0' <source folder> <destination folder> [tint color] [tint percent]"  >&2
	exit 1
}

if [ $# -lt 2 -o ! -d "$1" ] ; then
	print_usage_and_exit
fi

if [ ! -d "$2" ] ; then
	mkdir -p "$2" || exit 1
fi

imagefiles=`cd "$1" && find . -type f \( -iname "*.png" -o -iname "*.bmp" -o -iname "*.jpg" \)`
if [ -z "$imagefiles" ] ; then
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

# Determine tint color
tintcolor="white"
if [ $# -ge 3 -o ! -z "$3" ] ; then
	tintcolor="$3"
fi

# Determine tint percentage
tintpercent="75%"
if [ $# -ge 4 -o ! -z "$4" ] ; then
	tintpercent="$4"
fi

echo "$imagefiles" | while read i ; do
	mkdir -p "$2/"`dirname "$i"`
	convert "$1/$i" -fill "$tintcolor" -tint "$tintpercent" -background none "$2/$i"
done

sourcefiles=`cd "$1" && find . ! -type d | sort -u`
destfiles=`cd "$2" && find . ! -type d | sort -u`
if [ "$sourcefiles" != "$destfiles" ] ; then
	echo "Error: source and destinations folders have extra or missing files" >&2
	exit 1
fi

exit 0
