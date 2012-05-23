#!/bin/sh

if [ ! -r "$1" ] ; then
	echo "Usage: $0 <source file>" >&2
	exit 1
fi

sed 's/\\/\\\\/g' "$1" | \
sed "s/'/\\\\'/g" | \
sed -e ':a' -e 'N' -e '$!ba' -e 's/\n/\\n/g' | \
sed "s/^/escape('/" | \
sed "s/$/');/" | \
/System/Library/Frameworks/JavaScriptCore.framework/Resources/jsc || exit 1

exit 0
