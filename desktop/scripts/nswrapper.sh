#!/bin/sh

while [ "$1" != "" ] ; do
	/usr/bin/open "$1"
	shift
fi

exit 0
