#!/bin/sh

set -e

if [ ! -r "$1" ] ; then
	echo "Usage: $0 <source file>" >&2
	exit 1
fi

escape()
{
	sed 's/\\/\\\\/g' "$1" | \
	sed "s/'/\\\\'/g" | \
	sed -e ':a' -e 'N' -e '$!ba' -e 's/\n/\\n/g' | \
	sed "s/^/escape('/" | \
	sed "s/$/');/" | \
	/System/Library/Frameworks/JavaScriptCore.framework/Versions/A/Helpers/jsc
	return $?
}

escapedtext=`escape "$1"`
if [ "$?" != "0" -o -z "$escapedtext" ] ; then
	exit 1
fi

# jsc on macOS 12 adds leading and trailing double quotes so strip those out
echo "$escapedtext" | sed '/^undefined$/d' | sed 's/^"//' | sed 's/"$//'

exit 0
