#!/bin/sh
##########################################################################
#
#   $RCSfile$
#
#   $Revision$
#
#   last change: $Author$ $Date$
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU General Public License Version 2.1
#
#   Patrick Luby, June 2003
#
#   GNU General Public License Version 2.1
#   =============================================
#   Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
##########################################################################

# Reset PATH
PATH=/usr/bin:/bin:/usr/sbin:/sbin
export PATH

error()
{
    if [ ! -z "$@" ] ; then
        echo "Error: $@"
    fi
    echo "Usage: $0 [-h] [-repair]"
    exit 1
}

os=`uname`
apphome=`dirname "$0"`
sharebase="$apphome/../share"
userbase="$apphome/../user"
userinstall="$HOME/Library/$(PRODUCT_DIR_NAME)-$(OO_VERSION)/user"

# Make sure that this is not a botched installation
if [ ! -d "$apphome" ] ; then
    error "$apphome directory does not exist"
fi
if [ ! -d "$sharebase" ] ; then
    error "$sharebase directory does not exist"
fi
if [ ! -d "$userbase" ] ; then
    error "$userbase directory does not exist"
fi

# Parse arguments
locale=""
repair=""
while [ ! -z "$1" ] ; do
    case "$1" in
    -repair)
        repair="true"
        shift;;
    -h)
        error;;
    *)
        error "$1 argument is not recognized";;
    esac
done

# Create user installation directory
configdir="$userinstall/config"
registrydir="$userinstall/registry"
if [ ! -d "$configdir" -o ! -d "$registrydir" ]; then
    repair="true"
fi
if [ ! -z "$repair" ] ; then
    chmod -Rf u+rw "$userinstall"
    rm -Rf "$userinstall"
    mkdir -p "$userinstall"
    cp -Rf "$userbase"/* "$userinstall"
    chmod -Rf u+rw "$userinstall"
    if [ ! -d "$configdir" -o ! -d "$registrydir" ]; then
        rm -Rf "$userinstall"
        error "Installation of files in the $userinstall directory failed"
    fi
fi

# Create javarc file
sysclasspath=""
if [ ! -d "$apphome/classes" ] ; then
    error "$apphome/classes directory does not exist"
fi
for i in `cd "$apphome/classes" ; find . -name "*.jar"` ; do
    sysclasspath="$sysclasspath:$apphome/classes/$i"
done
sysclasspath=`printf "$sysclasspath" | sed 's#^:##'`
if [ "$os" = "Darwin" ] ; then
    # Turn off graphics acceleration
    printf "[Java]\nRuntimeLib=/System/Library/Frameworks/JavaVM.framework/JavaVM\ncom.apple.hwaccel=false\ncom.apple.hwaccellist=\n" > "$configdir/javarc"
else
    printf "[Java]\n" > "$configdir/javarc"
fi
printf "SystemClasspath=$sysclasspath\nJava=1\nJavaScript=1\nApplets=1\n-Xmx512m\n" >> "$configdir/javarc"
if [ ! -f "$configdir/javarc" ] ; then
    error "$configdir/javarc file could not be created"
fi

# Install application fonts
if [ "$os" = "Darwin" ] ; then
    appfontdir="$sharebase/fonts/truetype"
    userfontdir="$HOME/Library/Fonts"
    mkdir -p "$userfontdir"
    if [ -d "$appfontdir" -a -d "$userfontdir" ] ; then
        for i in `cd "$appfontdir" ; find . -name '*.ttf'` ; do
            if [ -L "$userfontdir/$i" ] ; then
                rm -f "$userfontdir/$i"
            fi
            if [ ! -f "$userfontdir/$i" ] ; then
                cp -f "$appfontdir/$i" "$userfontdir/$i"
            fi
        done
    fi
fi

exit 0
