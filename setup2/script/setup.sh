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
    exit 1
}

os=`uname`
apphome=`dirname "$0"`
sharebase="$apphome/../share"
userbase="$apphome/../user"
userlibrary="$HOME/Library/Preferences"
userinstall="$userlibrary/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)/user"


# Make sure the user's home directory exists and is writable
if [ ! -d "$userlibrary" ] ; then
    mkdir -p "$userlibrary"
fi
chmod -f u+w "$userlibrary"

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
        locale="$1";;
    -repair)
        repair="true";;
    -h)
        echo "Usage: $0 [-h] [-locale <locale>] [-repair]";;
    *)
        ;;
    esac
    shift
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
    find "$userinstall" -type d -exec chmod u+rwx {} \;
    chmod -Rf u+rw "$userinstall"
    # Make backup copy
    if [ -d "$userinstall" ] ; then
        if [ ! -z "`ls "$userinstall"`" ] ; then
            userinstallbak="$userinstall.backup.`date +%Y%m%d%H%M`"
            mkdir -p "$userinstallbak"
            ( cd "$userinstall" ; pax -r -w "." "$userinstallbak" )
        fi
    else
        rm -f "$userinstall"
    fi
    mkdir -p "$userinstall"
    # Make a clean copy of the registry directory and only copy missing
    # files in all other directories
    rm -Rf "$userinstall/registry"
    ( cd "$userbase" ; pax -r -w "registry" "$userinstall" )
    ( cd "$userbase" ; pax -r -w -k "." "$userinstall" )
    chmod -Rf u+rw "$userinstall"
    if [ ! -d "$configdir" -o ! -d "$registrydir" -o ! -d "$wordbookdir" ] ; then
        error "Installation of files in the $userinstall directory failed"
    fi
fi

# Set the locale
setupxml="$registrydir/Setup.xcu"
setupxmlbak="$setupxml.bak"
rm -f "$setupxmlbak"
if [ ! -f "$setupxml" ] ; then
    error "$setupxml file not found"
fi
if [ ! -f "$setupxmlbak" ] ; then
    # Begin multi-line pattern
    localepattern='/<prop oor:name="ooLocale" oor:type="xs:string">/{
N
s#<value.*$#<value>'"$locale"'</value>#
}'
    # End multi-line pattern

    sed -e "$localepattern" "$setupxml" > "$setupxmlbak"
    if [ $? -eq 0 -a -s "$setupxmlbak" ] ; then
        mv -f "$setupxmlbak" "$setupxml"
    else
        rm -f "$setupxmlbak"
    fi
fi

# Force registration dialog to appear at least once
commonxml="$registrydir/Office/Common.xcu"
commonxmlset="$commonxml.set"
commonxmlbak="$commonxml.bak"
rm -f "$commonxmlbak"
if [ ! -f "$commonxml" ] ; then
    error "$commonxml file not found"
fi
if [ ! -f "$commonxmlset" -a ! -f "$commonxmlbak" ] ; then
    jobsxml="$registrydir/Office/Jobs.xcu"
    jobsxmlbak="$jobsxml.bak"
    rm -f "$jobsxmlbak"
    if [ -f "$jobsxml" -a ! -f "$jobsxmlbak" ] ; then
        # Begin multi-line pattern
        defregpattern='/<node oor:name="RegistrationRequest">/{
:addline
N
s#<node .*</node>##
t
b addline
}'
        # End multi-line pattern

        sed -e "$defregpattern" "$jobsxml" > "$jobsxmlbak"
        if [ $? -eq 0 -a -s "$jobsxmlbak" ] ; then
            mv -f "$jobsxmlbak" "$jobsxml"
        else
            rm -f "$jobsxmlbak"
        fi
    fi

    # Begin multi-line pattern
    defregpattern='/<prop oor:name="RequestDialog" oor:type="xs:int">/{
N
s#<value.*$#<value>1</value>#
}'
    # End multi-line pattern

    sed -e "$defregpattern" "$commonxml" > "$commonxmlbak"
    if [ $? -eq 0 -a -s "$commonxmlbak" ] ; then
        mv -f "$commonxmlbak" "$commonxml"
        touch -f "$commonxmlset"
    else
        rm -f "$commonxmlbak"
    fi
fi

# Create user dictionary.lst file
lang=`echo "$locale" | awk -F- '{ print $1 }'`
sharedictdir="$sharebase/dict/ooo"
userdictlst="$wordbookdir/dictionary.lst"
if [ -d "$sharedictdir" ] ; then
    sharedictlst="$sharedictdir/dictionary.lst"
    if [ -r "$sharedictlst" ] ; then
        ( cat /dev/null "$userdictlst" ; grep -E '[^][#:space:]*(DICT|HYPH|THES)[[:space:]]*'"$lang"'[[:space:]]' "$sharedictlst" ) 2>/dev/null | sed 's#^[#[:space:]]*##' | sort -u > "$userdictlst.tmp"
        if [ -s "$userdictlst.tmp" ] ; then
            lasttype=
            lastlang=
            lastcountry=
            while read type lang country file ; do
                if [ "$type $lang $country" = "$lasttype $lastlang $lastcountry" ] ; then
                    continue;
                fi
                echo "$type $lang $country $file"
                lasttype="$type"
                lastlang="$lang"
                lastcountry="$country"
            done < "$userdictlst.tmp" > "$userdictlst"
        fi
        rm -f "$userdictlst.tmp"
    fi
    for i in `cd "$sharedictdir" ; find . -name "*$lang*"` ; do
        if [ ! -r "$wordbookdir/$i" ] ; then
            ln -sf "$sharedictdir/$i" "$wordbookdir/$i"
        fi
    done
fi

# Make locale the default document language
linguxml="$registrydir/Office/Linguistic.xcu"
linguxmlset="$linguxml.set"
linguxmlbak="$linguxml.bak"
rm -f "$linguxmlbak"
if [ ! -f "$linguxml" ] ; then
    error "$linguxml file not found"
fi
if [ ! -f "$linguxmlset" -a ! -f "$linguxmlbak" ] ; then
    # Match the locale to one of the installed locales
    locales='$(LANGUAGE_NAMES)'
    if [ -f "$userdictlst" ] ; then
        locales="$locales "`awk '{ print $2 "-" $3 }' "$userdictlst"`
    fi
    matchedlocale=""
    for i in $locales ; do
        if [ "$locale" = "$i" ] ; then
            matchedlocale="$i"
            break
        fi
    done
    if [ -z "$matchedlocale" ] ; then
        for i in $locales ; do
            ilang=`echo "$i" | awk -F- '{ print $1 }'`
            if [ "$lang" = "$ilang" ] ; then
                matchedlocale="$i"
                break
            fi
        done
    fi
    if [ -z "$matchedlocale" ] ; then
        lang="US"
        locale="en-$lang"
    else
        locale="$matchedlocale"
    fi

    # Begin multi-line pattern
    deflocalepattern='/<prop oor:name="DefaultLocale" oor:type="xs:string">/{
N
s#<value.*$#<value>'"$locale"'</value>#
}'
    # End multi-line pattern

    sed -e "$deflocalepattern" "$linguxml" > "$linguxmlbak"
    if [ $? -eq 0 -a -s "$linguxmlbak" ] ; then
        mv -f "$linguxmlbak" "$linguxml"
        touch -f "$linguxmlset"
    else
        rm -f "$linguxmlbak"
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
    javavm="/System/Library/Frameworks/JavaVM.framework/Versions/A/JavaVM"
    if [ ! -f "$javavm" ] ; then
        javavm="/System/Library/Frameworks/JavaVM.framework/JavaVM"
        if [ ! -f "$javavm" ] ; then
            error "$javavm file does not exist"
        fi
    fi
    # Force vcl.jar into bootstrap classpath so that we are sure that our
    # classes are loaded
    printf "[Java]\nRuntimeLib=$javavm\n-Xbootclasspath/a:$apphome/classes/vcl.jar\n" > "$configdir/javarc"
else
    printf "[Java]\n" > "$configdir/javarc"
fi
printf "SystemClasspath=$sysclasspath\nJava=1\nJavaScript=1\nApplets=1\n-Xrs\n-Xmx256m\n-XX:+UseParallelGC\n" >> "$configdir/javarc"
if [ ! -f "$configdir/javarc" -o ! -s "$configdir/javarc" ] ; then
    error "$configdir/javarc file could not be created"
fi

# Make autocorrect files available
shareautocorrdir="$sharebase/autocorr"
userautocorrdir="$userinstall/autocorr"
mkdir -p "$userautocorrdir"
if [ -d "$shareautocorrdir" -a -d "$userautocorrdir" ] ; then
    chmod -Rf u+rwx "$userautocorrdir"
    for i in `cd "$shareautocorrdir" ; find . -name '*.dat'` ; do
        if [ -z "$i" ] ; then
            continue;
        fi
        if [ ! -f "$userautocorrdir/$i" ] ; then
            cat /dev/null "$shareautocorrdir/$i" > "$userautocorrdir/$i"
        fi
    done
fi

# Fix bug 154 by checking if the installation location has changed
lastcontentshomefile="$userinstall/.lastcontentshome"
contentshome=`dirname "$apphome"`
if [ -d "$contentshome" -a -r "$lastcontentshomefile" ] ; then
    lastcontentshome=`cat /dev/null "$lastcontentshomefile"`
    rm -f "$lastcontentshomefile"
    if [ ! -z "$lastcontentshome" ] ; then
        for i in `cd "$userinstall" ; find . -type f -name "*.x*"` ; do
            rm -f "$userinstall/$i.tmp"
            if [ ! -f "$userinstall/$i.tmp" ] ; then
                sed -e "s#$lastcontentshome#$contentshome#g" "$userinstall/$i" > "$userinstall/$i.tmp"
                if [ $? -eq 0 -a -s "$userinstall/$i.tmp" ] ; then
                    mv -f "$userinstall/$i.tmp" "$userinstall/$i"
                else
                    rm -f "$userinstall/$i.tmp"
                fi
            fi
        done
    fi
fi
echo "$contentshome" > "$lastcontentshomefile"

# Make sure that there is a /tmp directory
if [ "$os" = "Darwin" ] ; then
    if [ ! -d "/tmp" -a -d "/private/tmp" ] ; then
        ln -sf "private/tmp" "/tmp"
    fi
fi

sync

checkforpatches()
{
    soffice=`dirname "$0"`/soffice.bin
    if [ ! -x "$soffice" -o ! -x "/usr/sbin/scutil" -o ! -x "/usr/bin/curl" ] ; then
        return 1
    fi

    patchfileurl="$(PRODUCT_PATCH_CHECK_URL)"
    patchdownloadurl="$(PRODUCT_PATCH_DOWNLOAD_URL)"
    lastcheckfile="$userinstall/.lastpatchcheck"
    if [ ! -r "$lastcheckfile" -o -z "`find "$lastcheckfile" -mtime -6 -o -mtime -5 -o -mtime -4 -o -mtime -3 -o -mtime -2 -o -mtime -1 -o -mtime 0`" ] ; then
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

        content=
        if [ -z "$httpproxy" ] ; then
            content=`curl --connect-timeout 30 "$patchfileurl" 2>/dev/null`
        else
            content=`curl --proxy "$httpproxy" --connect-timeout 30 "$patchfileurl" 2>/dev/null`
        fi

        # Show patch download URL
        newproductkey=`echo "$content" | grep "^ProductKey=" | awk -F= '{ print $2 }'`
        newproductpatch=`echo "$content" | grep "^ProductPatch=" | awk -F= '{ print $2 }'`
        oldproductkey=`grep "^ProductKey=" "$apphome/bootstraprc" | awk -F= '{ print $2 }'`
        oldproductpatch=`grep "^ProductPatch=" "$apphome/bootstraprc" | awk -F= '{ print $2 }'`
        if [ "$newproductkey" != "$oldproductkey" -o "$newproductpatch" != "$oldproductpatch" ] ; then 
            sleep 15
            open "$patchdownloadurl"
        fi

        # Cache the last check date
        touch -f "$lastcheckfile"
    fi
}

# Check for patches
if [ "$os" = "Darwin" -a ! -f "$apphome/.nocheckforpatches" ] ; then
    checkforpatches >/dev/null 2>&1 &
fi

exit 0
