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

os=`uname`
apphome=`dirname "$0"`
userbase="$apphome/../user"
sharebase="$apphome/../share"
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

# Match the locale to one of the installed locales
lang=`echo "$locale" | awk -F- '{ print $1 }'`
country=`echo "$locale" | awk -F- '{ print $2 }'`
locales="$(LANGUAGE_NAMES)"
matchedlocale=""
for i in $locales ; do
    if [ "$locale" = "$i" ] ; then
        matchedlocale="$i"
        break
    fi
done
if [ -z "$matchedlocale" ] ; then
    if [ -z "$country" ] ; then
        for i in $locales ; do
            ilang=`echo "$i" | awk -F- '{ print $1 }'`
            if [ "$lang" = "$ilang" ] ; then
                matchedlocale="$i"
                break
            fi
        done
    else
        for i in $locales ; do
            if [ "$lang" = "$i" ] ; then
                matchedlocale="$i"
                break
            fi
        done
    fi
fi
if [ -z "$matchedlocale" ] ; then
    locale="en-US"
else
    locale="$matchedlocale"
fi

configdir="$userinstall/config"
xmldir="$configdir/registry/instance/org/openoffice"
xmltemplatedir="$configdir/registry/template/org/openoffice"

# Create user installation directory
if [ ! -d "$userinstall" ] ; then
    repair="true"
    mkdir -p "$userinstall"
fi
if [ ! -z "$repair" ] ; then
    chmod -Rf u+rw "$userinstall"
    cp -Rf "$userbase"/* "$userinstall"
    chmod -Rf u+rw "$userinstall"
fi

# Copy and edit required files
if [ ! -d "$xmldir" ] ; then
    error
fi
if [ ! -d "$xmltemplatedir" ] ; then
    error
fi
for i in `cd "$xmltemplatedir" ; find . ! -type d` ; do
    if [ ! -z "$repair" -o ! -f "$xmldir/$i" ] ; then
        sed 's#>USER_INSTALL_DIR<#>'"$userinstall"'<#g' "$xmltemplatedir/$i" | sed 's#>LOCALE<#>'"$locale"'<#g' | sed 's#>NSWRAPPER_PATH<#>'"$apphome/nswrapper"'<#g' | sed 's#>CURRENT_DATE<#>'`date +%d.%m.%Y/%H.%M.%S`'<#g' > "$xmldir/$i"
    fi
done

# Set the locale
setupxml="$xmldir/Setup.xml"
if [ ! -f "$setupxml" ] ; then
    error
fi
setupxmlbak="$setupxml.bak"
rm -f "$setupxmlbak"
if [ ! -f "$setupxmlbak" ] ; then
    cp -f "$setupxml" "$setupxmlbak"
    sed 's#>.*</ooSetupInstallPath>#>'"$userinstall"'</ooSetupInstallPath>#g' "$setupxmlbak" | sed 's#>.*</ooLocale>#>'"$locale"'</ooLocale>#g' > "$setupxml"
    rm -f "$setupxmlbak"
fi

# Make locale the default document language
linguxml="$xmldir/Office/Linguistic.xml"
if [ ! -f "$linguxml" ] ; then
    error
fi
linguxmlbak="$linguxml.bak"
if [ ! -f "$linguxmlbak" ] ; then
    cp -f "$linguxml" "$linguxmlbak"
    sed 's#<DefaultLocale cfg:type="string"/>#<DefaultLocale cfg:type="string">'"$locale"'</DefaultLocale>#g' "$linguxmlbak" > "$linguxml"
fi

if [ "$os" = "Darwin" ] ; then
    # Make Mac look and feel the default if none is set
    commonxml="$xmldir/Office/Common.xml"
    if [ ! -f "$commonxml" ] ; then
        error
    fi
    grep -q '</LookAndFeel>' "$commonxml"
    if [ "$?" != "0" ] ; then
        commonxmlbak="$commonxml.bak"
        cp -f "$commonxml" "$commonxmlbak"
        grep -q '</View>' "$commonxml"
        if [ "$?" != "0" ] ; then
            sed 's#</Common>#<View><LookAndFeel cfg:type="short">4</LookAndFeel></View></Common>#g' "$commonxmlbak" > "$commonxml"
        else
            sed 's#</View>#<LookAndFeel cfg:type="short">4</LookAndFeel></View>#g' "$commonxmlbak" > "$commonxml"
        fi
        rm -f "$commonxmlbak"
    fi
fi

# Create javarc file
sysclasspath=""
if [ ! -d "$apphome/classes" ] ; then
    error
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

# Install application fonts
if [ "$os" = "Darwin" ] ; then
    appfontdir="$sharebase/fonts/truetype"
    userfontdir="$HOME/Library/Fonts"
    mkdir -p "$userfontdir"
    if [ ! -d "$appfontdir" -o ! -d "$userfontdir" ] ; then
        error
    fi
    for i in `cd "$appfontdir" ; find . -name '*.ttf'` ; do
        if [ -L "$userfontdir/$i" ] ; then
            rm -f "$userfontdir/$i"
        fi
        if [ ! -f "$userfontdir/$i" ] ; then
            cp -f "$appfontdir/$i" "$userfontdir/$i"
        fi
    done
fi

exit 0
