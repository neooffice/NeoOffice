#!/bin/sh

if [ `uname` = "Darwin" ] ; then
	while [ "$1" != "" ] ; do
		URL=`echo "$1" | sed 's#^/#file:///#'`
		if [ "$1" != "" ] ; then
			/usr/bin/open "$URL"
		fi
		shift
	done
else
	# if mozilla is not found, specify full path here
	MOZILLA=mozilla

	if ${MOZILLA} -remote "openURL($1,new-window)" 2>&1 | egrep -si "not running on display"; then
		${MOZILLA} $1
	fi
fi

exit 0
