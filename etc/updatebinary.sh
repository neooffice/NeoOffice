#!/bin/sh

set -e

if [ ! -w "$1" ] ; then
	echo "Usage: $0 <source file>" >&2
	exit 1
fi

for i in `otool -L "$1" | awk '{ print $1 }' | grep -e '^@executable_path/' -e '^@loader_path/'`
do
	install_name_tool -change "$i" '@executable_path/'`basename "$i"` "$1"
done

exit 0
