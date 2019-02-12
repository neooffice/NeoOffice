#!/bin/sh

if [ ! -d "../$1" -o ! -f "$2" ] ; then
echo "Usage: $0 <module> <file>" >&2
exit 1
fi

diff -dup "/Volumes/Sierra/NeoBuilds/libreoffice-4.4.7.2/$1/$2" "$2"
cp "../build/libreoffice-5.4.7.2/$1/$2" "$2"
