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
	# if netscape is not found, specify full path here
	NETSCAPE=netscape

	if ${NETSCAPE} -remote "openURL($1,new-window)" 2>&1 | egrep -si "not running on display"; then
		${NETSCAPE} $1
	fi
fi

exit 0
