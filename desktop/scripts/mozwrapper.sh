#!/bin/sh

while [ "$1" != "" ] ; do
	URL=`echo "$1" | sed 's#^/#file:///#'`
	if [ "$1" != "" ] ; then
		/usr/bin/open "$URL"
	fi
	shift
fi

exit 0
