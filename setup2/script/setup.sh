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

# Reset PATH
PATH=/usr/bin:/bin:/usr/sbin:/sbin
export PATH

error()
{
    if [ ! -z "$@" ] ; then
        echo "Error: $@"
    fi
    echo "Usage: $0 [-h] [-locale <locale>] [-repair]"
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
locale="en-US"
repair=""
while [ ! -z "$1" ] ; do
    case "$1" in
    -locale)
        shift
        locale="$1"
        shift;;
    -repair)
        repair="true"
        shift;;
    -h)
        error;;
    *)
        shift;;
    esac
done

# Create user installation directory
configdir="$userinstall/config"
registrydir="$userinstall/registry/data/org/openoffice"
wordbookdir="$userinstall/wordbook"
if [ ! -d "$configdir" -o ! -d "$registrydir" -o ! -d "$wordbookdir" ] ; then
    repair="true"
    mkdir -p "$userinstall"
fi
if [ ! -z "$repair" ] ; then
    chmod -Rf u+rw "$userinstall"
    rm -Rf "$userinstall"
    mkdir -p "$userinstall"
    ( cd "$userbase" ; tar cf - * ) | ( cd "$userinstall" ; tar xf - )
    chmod -Rf u+rw "$userinstall"
    if [ ! -d "$configdir" -o ! -d "$registrydir" -o ! -d "$wordbookdir" ] ; then
        rm -Rf "$userinstall"
        error "Installation of files in the $userinstall directory failed"
    fi
fi

# Set the locale
setupxml="$registrydir/Setup.xcu"
if [ ! -f "$setupxml" ] ; then
    error
fi
setupxmlbak="$setupxml.bak"
rm -f "$setupxmlbak"
if [ ! -f "$setupxmlbak" ] ; then
    cat /dev/null "$setupxml" > "$setupxmlbak"

    # Begin multi-line pattern
    localepattern='/<prop oor:name="ooLocale" oor:type="xs:string">/{
N
s#<value.*$#<value>'"$locale"'</value>#
}'
    # End multi-line pattern

    sed -e "$localepattern" "$setupxmlbak" > "$setupxml"
    rm -f "$setupxmlbak"
fi

# Make locale the default document language
lang=`echo "$locale" | awk -F- '{ print $1 }'`
linguxml="$registrydir/Office/Linguistic.xcu"
if [ ! -f "$linguxml" ] ; then
    error
fi
linguxmlset="$linguxml.set"
linguxmlbak="$linguxml.bak"
rm -f "$linguxmlbak"
if [ ! -f "$linguxmlset" -a ! -f "$linguxmlbak" ] ; then
    # Match the locale to one of the installed locales
    locales='$(LANGUAGE_NAMES)'
    country=`echo "$locale" | awk -F- '{ print $2 }'`
    matchedlocale=""
    for i in $locales ; do
        if [ "$locale" = "$i" ] ; then
            matchedlocale="$i"
            break
        fi
    done
    if [ -z "$matchedlocale" ] ; then
        country=""
        for i in $locales ; do
            ilang=`echo "$i" | awk -F- '{ print $1 }'`
            if [ "$lang" = "$ilang" ] ; then
                matchedlocale="$i"
                break
            fi
        done
    fi
    if [ -z "$matchedlocale" ] ; then
        lang="en"
        country="US"
        locale="$lang-$country"
    else
        locale="$matchedlocale"
    fi

    cat /dev/null "$linguxml" > "$linguxmlbak"

    # Begin multi-line pattern
    deflocalepattern='/<prop oor:name="DefaultLocale" oor:type="xs:string">/{
N
s#<value.*$#<value>'"$locale"'</value>#
}'
    # End multi-line pattern

    sed -e "$deflocalepattern" "$linguxmlbak" > "$linguxml"
    rm -f "$linguxmlbak"
    touch -f "$linguxmlset"
fi

# Create user dictionary.lst file
sharedictdir="$sharebase/dict/ooo"
if [ -d "$sharedictdir" ] ; then
    userdictlst="$wordbookdir/dictionary.lst"
    sharedictlst="$sharedictdir/dictionary.lst"
    if [ -r "$sharedictlst" ] ; then
        ( cat /dev/null "$userdictlst" ; grep -E '[^][#:space:]*(DICT|HYPH|THES)[[:space:]]*'"$lang"'[[:space:]]' "$sharedictlst" ) 2>/dev/null | sed 's#^[#[:space:]]*##' > "$userdictlst.tmp"
        sort -u "$userdictlst.tmp" > "$userdictlst"
        rm -f "$userdictlst.tmp"
    fi
    for i in `cd "$sharedictdir" ; find . -name "*$lang*"` ; do
        if [ ! -r "$wordbookdir/$i" ] ; then
            ln -sf "$sharedictdir/$i" "$wordbookdir/$i"
        fi
    done
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
printf "SystemClasspath=$sysclasspath\nJava=1\nJavaScript=1\nApplets=1\n-Xrs\n-Xmx512m\n" >> "$configdir/javarc"
if [ ! -f "$configdir/javarc" ] ; then
    error "$configdir/javarc file could not be created"
fi

# Install application fonts
if [ "$os" = "Darwin" ] ; then
    userfontdir="$HOME/Library/Fonts"
    mkdir -p "$userfontdir"
    if [ -d "$userfontdir" ] ; then
        appfontdir="$userinstall/fonts"
        if [ -d "$appfontdir" ] ; then
            for i in `cd "$appfontdir" ; find . -name '*.ttf' -o -name '*.TTF'` ; do
                if [ -z "$i" ] ; then
                    continue;
                fi
                if [ -L "$userfontdir/$i" ] ; then
                    rm -f "$userfontdir/$i"
                fi
                if [ ! -f "$userfontdir/$i" ] ; then
                    cat /dev/null "$appfontdir/$i" > "$userfontdir/$i"
                fi
            done
        fi
        appfontdir="$sharebase/fonts/truetype"
        if [ -d "$appfontdir" ] ; then
            for i in `cd "$appfontdir" ; find . -name '*.ttf' -o -name '*.TTF'` ; do
                if [ -z "$i" ] ; then
                    continue;
                fi
                if [ -L "$userfontdir/$i" ] ; then
                    rm -f "$userfontdir/$i"
                fi
                if [ ! -f "$userfontdir/$i" ] ; then
                    cat /dev/null "$appfontdir/$i" > "$userfontdir/$i"
                fi
            done
        fi
    fi
fi

sync

checkforpatches()
{
    soffice=`dirname "$0"`/soffice.bin
    if [ ! -x "$soffice" ] ; then
        return 1
    fi

    patchfileurl="$(PRODUCT_PATCH_CHECK_URL)"
    patchdownloadurl="$(PRODUCT_PATCH_DOWNLOAD_URL)"
    lastcheckfile="$userinstall/.lastpatchcheck"
    status=
    if [ ! -r "$lastcheckfile" ] ; then
        touch -r "$apphome" "$lastcheckfile"
    fi
    if [ -r "$lastcheckfile" -a -z "`find "$lastcheckfile" -mtime -7 -o -mtime -6 -o -mtime -5 -o -mtime -4 -o -mtime -3 -o -mtime -2 -o -mtime -1 -o -mtime 0`" ] ; then
        proxies=`scutil << !
open
get "State:/Network/Global/Proxies"
d.show
quit
!`
        httpproxy=
        if echo "$proxies" | grep 'HTTPEnable : 1' ; then
            httpproxy=`echo "$proxies" | grep 'HTTPProxy :' | awk '{ print $NF }'`
            httpport=`echo "$proxies" | grep 'HTTPPort :' | awk '{ print $NF }'`
            if [ ! -z "$httpport" ] ; then
                httpproxy="$httpproxy:$httpport"
            fi
        fi
        if [ -z "$httpproxy" ] ; then
            status=`curl --connect-timeout 30 --time-cond "$lastcheckfile" --head "$patchfileurl" 2>/dev/null | head -1 | awk '{ print $2 }'`
        else
            status=`curl --proxy "$httpproxy" --connect-timeout 30 --time-cond "$lastcheckfile" --head "$patchfileurl" 2>/dev/null | head -1 | awk '{ print $2 }'`
        fi

        # Cache the last check date
        touch -f "$lastcheckfile"
    fi

    # Show patch download URL
    if [ -w "$lastcheckfile" -a "$status" = "200" ] ; then
        sleep 15
        open "$patchdownloadurl"
    fi
}

# Check for patches
if [ "$os" = "Darwin" ] ; then
    checkforpatches >/dev/null 2>&1 &
fi

exit 0
