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
    if [ ! -z "$1" ] ; then
        echo "Error: $1"
    fi
    echo "Usage: $0 [-h] -locale <locale> [-repair]"
    exit 1
}

apphome=`dirname "$0"`
userbase="$apphome/../user"
userinstall="$HOME/Library/NeoOfficeJ/user"

# Make sure that this is not a botched installation
if [ ! -d "$apphome" ] ; then
    error "$apphome directory does not exist"
fi
if [ ! -d "$userbase" ] ; then
    error "$userbase directory does not exist"
fi

# Parse arguments
locale=""
repair=""
while [ ! -z "$1" ] ; do
    case "$1" in
    -locale)
        if [ -z "$2" ] ; then
            error "-locale argument must be followed by a locale"
        fi
        locale="$2"
        shift 2;;
    -repair)
        repair="true"
        shift;;
    -h)
        error;;
    *)
        error "$1 argument is not recognized";;
    esac
done

# Make sure that a locale was specified
if [ -z "$locale" ] ; then
     error "-locale argument missing"
fi

configdir="$userinstall/config"
xmldir="$configdir/registry/instance/org/openoffice"
xmltemplatedir="$configdir/registry/template/org/openoffice"

# Create user installation directory
if [ ! -d "$userinstall" ] ; then
    repair="true"
    mkdir -p "$userinstall"
fi
chmod -Rf u+rw "$userinstall"
if [ $? != 0 ]; then
    error
fi
cp -Rf "$userbase"/* "$userinstall"
if [ $? != 0 ]; then
    error
fi
chmod -Rf u+rw "$userinstall"
if [ $? != 0 ]; then
    error
fi

# Copy and edit required files
if [ ! -d "$xmldir" ] ; then
    error
fi
if [ ! -d "$xmltemplatedir" ] ; then
    error
fi
for i in `cd "$xmltemplatedir" ; find . ! -type d` ; do
    if [ ! -z "$repair" -o "$i" = "./Setup.xml" ] ; then
        sed 's#>USER_INSTALL_DIR<#>'"$userinstall"'<#g' "$xmltemplatedir/$i" | sed 's#>LOCALE<#>'"$locale"'<#g' | sed 's#>NSWRAPPER_PATH<#>'"$apphome/nswrapper"'<#g' | sed 's#>CURRENT_DATE<#>'`date +%d.%m.%Y/%H.%M.%S`'<#g' > "$xmldir/$i"
        if [ $? != 0 ]; then
            error
        fi
    fi
done
sysclasspath=""
if [ ! -d "$apphome/classes" ] ; then
    error
fi
for i in `cd "$apphome/classes" ; find . -name "*.jar"` ; do
    sysclasspath="$sysclasspath:$apphome/classes/$i"
done
sysclasspath=`printf "$sysclasspath" | sed 's#^:##'`
if [ $? != 0 ]; then
    error
fi
printf "[Java]\nRuntimeLib=/System/Library/Frameworks/JavaVM.framework/JavaVM\nSystemClasspath=$sysclasspath\ncom.apple.hwaccel=false\ncom.apple.hwaccellist=\nJava=1\nJavaScript=1\nApplets=1\n\n" > "$configdir/javarc"
if [ $? != 0 ]; then
    error
fi

exit 0
