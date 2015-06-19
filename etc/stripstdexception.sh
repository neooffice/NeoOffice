#!/bin/sh -e

if [ ! -r "$1" ] ; then
	echo "Usage: $0 <source file>" >&2
	exit 1
fi

sed 's/^\(.*\), std::exception\(.*\)$/#if SUPD == 310\
\1\2\
#else	\/\/ SUPD == 310\
\1, std::exception\2\
#endif	\/\/ SUPD == 310/g' "$1"

exit 0
