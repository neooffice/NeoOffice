#!/bin/sh

cmd=`dirname "$0"`/../MacOS/soffice.bin
exec "$cmd" -unopkg "$@"
