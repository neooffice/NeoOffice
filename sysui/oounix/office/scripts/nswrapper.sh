#!/bin/sh

if [ "$1" = "" ]; then
	exit 0
fi

if [ -x /usr/bin/osascript ]; then
	# Send an open document event to the default browser
	exec /usr/bin/osascript -e 'open location "'"$1"'"'
fi

exit 0
