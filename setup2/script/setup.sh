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
#   Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

apphome=`dirname "$0"`
userbase="$apphome/../user"
userinstall="$HOME/Library/NeoOfficeJ/user"

# Make sure that this is not a botched installation
if [ ! -d "$apphome" -o ! -d "$userbase" ] ; then
    exit 1;
fi

configdir="$userinstall/config"
xmldir="$configdir/registry/instance/org/openoffice"

# Create user installation directory
if [ ! -d "$userinstall" ] ; then
    mkdir -p "$userinstall"
fi
chmod -Rf u+rw "$userinstall"
if [ $? != 0 ]; then
    exit 1;
fi
cp -Rf "$userbase"/* "$userinstall"
if [ $? != 0 ]; then
    exit 1;
fi
chmod -Rf u+rw "$userinstall"
if [ $? != 0 ]; then
    exit 1;
fi

# Copy and edit required files
if [ ! -d "$xmldir" ] ; then
    exit 1;
fi
for i in `cd "$xmldir" ; find . ! -type d` ; do
    sed 's#>USER_INSTALL_DIR<#>'"$userinstall"'<#g' "$xmldir/$i" | sed 's#>CURRENT_DATE<#>'`date +%d.%m.%Y/%H.%M.%S`'<#g' > "$xmldir/$i.tmp"
   	if [ $? != 0 ]; then
        exit 1;
    fi
    mv "$xmldir/$i.tmp" "$xmldir/$i"
   	if [ $? != 0 ]; then
        exit 1;
    fi
done
sysclasspath=""
if [ ! -d "$apphome/classes" ] ; then
    exit 1;
fi
for i in `cd "$apphome/classes" ; find . -name "*.jar"` ; do
    sysclasspath="$sysclasspath:$apphome/classes/$i"
done
sysclasspath=`printf "$sysclasspath" | sed 's#^:##'`
if [ $? != 0 ]; then
    exit 1;
fi
printf "[Java]\nRuntimeLib=/System/Library/Frameworks/JavaVM.framework/JavaVM\nSystemClasspath=$sysclasspath\ncom.apple.hwaccel=false\ncom.apple.hwaccellist=\nJava=1\nJavaScript=1\nApplets=1\n\n" > "$configdir/javarc"
if [ $? != 0 ]; then
    exit 1;
fi

exit 0
