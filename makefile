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

# Set the shell to tcsh since the OpenOffice.org build requires it
SHELL:=/bin/tcsh

# Build location macros
BUILD_HOME:=build
DIC_HOME:=dic
INSTALL_HOME:=install
SOURCE_HOME:=source
OO_PATCHES_HOME:=patches/openoffice
OO_ENV_X11:=$(BUILD_HOME)/MacosxEnv.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacosxEnvJava.Set
OO_LANGUAGES=ALL
OO_DIC_URL:=http://ftp.services.openoffice.org/pub/OpenOffice.org/contrib/dictionaries

# Product information
PRODUCT_NAME=NeoOffice/J
PRODUCT_DIR_NAME=NeoOfficeJ
# Important: Note that there are escape characters in the PRODUCT_NAME for the
# UTF-8 trademark symbol. Don't replace these with "\x##" literal strings!
PRODUCT_TRADEMARKED_NAME=NeoOffice®/J
PRODUCT_VERSION=0.8.4
PRODUCT_FILETYPE=NO%F
PRODUCT_INSTALL_URL=http://www.planamesa.com/neojava/download.php\\\#install
PRODUCT_BUILD_URL=http://www.planamesa.com/neojava/build.php

# CVS macros
OO_CVSROOT:=:pserver:anoncvs@anoncvs.services.openoffice.org:/cvs
OO_PACKAGES:=MathMLDTD UnoControls XmlSearch apiwww autodoc automation basctl basic berkeleydb bridges chaos codemaker comphelper config_office configmgr connectivity cosv cppu cppuhelper cpputools dbaccess desktop dmake dtrans eventattacher expat ext_log4j extensions external extras fileaccess forms fpicker framework freetype goodies helpcontent i18n i18n_simple i18npool idl idlc inet instsetoo io javaunohelper jtools jurt jut ldapber lingu lingucomponent linguistic moz nas neon netbeans_integration odk offapi officecfg offmgr offuh package padmin product psprint rdbmaker readlicense readlicense_oo registry remotebridges res ridljar rsc sablot sal salhelper sandbox sax sc scaddins sch scp scptools sd setup2 sfx2 shell sj2 so3 solenv soltools sot starmath stlport stoc store svtools svx sw sysui toolkit tools transex3 ucb ucbhelper udkapi udkwww udm unoil unotools unzip uui vcl vos wizards xml2cmp xmlhelp xmloff xmlscript xmlwww zlib
OO_TAG:=OOO_STABLE_1_PORTS
NEO_CVSROOT:=:pserver:anoncvs@anoncvs.neooffice.org:/cvs
NEO_PACKAGE:=NeoOfficeJ
NEO_TAG:=NeoOfficeJ-0_8_4

all: build.all

# Create the build directory and checkout the OpenOffice.org sources
build.oo_checkout:
	mkdir -p "$(BUILD_HOME)"
# The OOo cvs server gets messed up with tags so we need to do a little trick
# to get the checkout to work
	rm -Rf "$(BUILD_HOME)/tmp"
	mkdir -p "$(BUILD_HOME)/tmp"
	cd "$(BUILD_HOME)/tmp" ; cvs -d "$(OO_CVSROOT)" co MathMLDTD ; cd MathMLDTD ; cvs update -d -r "$(OO_TAG)"
	rm -Rf "$(BUILD_HOME)/tmp"
# Do the real checkout
	cd "$(BUILD_HOME)" ; cvs -d "$(OO_CVSROOT)" co -r "$(OO_TAG)" $(OO_PACKAGES)
	chmod -Rf u+w "$(BUILD_HOME)"
	touch "$@"

build.oo_patches: build.oo_checkout \
	build.oo_external_patch
	touch "$@"

build.oo_odk_patches: build.oo_checkout \
	build.oo_odk_patch \
	build.oo_product_patch
	touch "$@"

build.oo_external_patch: $(OO_PATCHES_HOME)/external.patch build.oo_checkout
	chmod -Rf u+w "$(BUILD_HOME)/external/gpc"
	gnutar zxf "$(OO_PATCHES_HOME)/gpc231.tar.Z" -C "$(BUILD_HOME)/external/gpc"
	chmod -Rf u+w "$(BUILD_HOME)/external/gpc"
	mv -f "$(BUILD_HOME)/external/gpc/gpc231"/* "$(BUILD_HOME)/external/gpc"
	rm -Rf "$(BUILD_HOME)/external/gpc/gpc231"
	-( cd "$(BUILD_HOME)/external" ; patch -R -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	cp "$(OO_PATCHES_HOME)/dlcompat-20020709.tar.gz" "$(BUILD_HOME)/external/download"
	cp "$(OO_PATCHES_HOME)/dlcompat.pat.tar.gz" "$(BUILD_HOME)/external/dlcompat"
	( cd "$(BUILD_HOME)/external" ; patch -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.oo_%_patch: $(OO_PATCHES_HOME)/%.patch build.oo_checkout
	-( cd "$(BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -R -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	( cd "$(BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.configure: build.oo_patches
	cd "$(BUILD_HOME)/config_office" ; autoconf
	( cd "$(BUILD_HOME)/config_office" ; ./configure CC=cc --with-x --with-lang="$(OO_LANGUAGES)" )
	( cd "$(BUILD_HOME)" ; ./bootstrap )
	touch "$@"

build.oo_all: build.configure
	source "$(OO_ENV_X11)" ; cd "$(BUILD_HOME)/instsetoo" ; `alias build` -all $(OO_BUILD_ARGS)
	touch "$@"

build.oo_odk_all: build.configure build.oo_all build.oo_odk_patches
# Building odk will fail unless we rebuild product and its dependencies first
	source "$(OO_ENV_X11)" ; cd "$(BUILD_HOME)/product" ; `alias build` -all $(OO_BUILD_ARGS)
	source "$(OO_ENV_X11)" ; cd "$(BUILD_HOME)/odk" ; `alias build` -all $(OO_BUILD_ARGS)
	touch "$@"

build.neo_configure: build.oo_all
	rm -f "$(OO_ENV_JAVA)"
	sed 's#^setenv GUIBASE .*$$#setenv GUIBASE "java"#' "$(OO_ENV_X11)" | sed 's#^setenv ENVCDEFS "#&-DUSE_JAVA#' | sed 's#^setenv CLASSPATH .*$$#setenv CLASSPATH "$$SOLARVER/$$UPD/$$INPATH/bin/vcl.jar"#' | sed 's#^setenv DELIVER .*$$#setenv DELIVER "true"#' | sed 's#^alias deliver .*$$#alias deliver "echo The deliver command has been disabled"#' > "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_NAME '$(PRODUCT_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_DIR_NAME '$(PRODUCT_DIR_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_TRADEMARKED_NAME '$(PRODUCT_TRADEMARKED_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_VERSION '$(PRODUCT_VERSION)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_FILETYPE '$(PRODUCT_FILETYPE)'" >> "$(OO_ENV_JAVA)"
# OOo 1.0.x cannot support RTL languages properly
	echo "unsetenv RES_ARAB" >> "$(OO_ENV_JAVA)"
	touch "$@"

build.neo_%_patch: % build.neo_configure
	cd "$<" ; sh -e -c 'for i in `cd "$(PWD)/$(BUILD_HOME)/$<" ; find . -type d | grep -v /CVS$$ | grep -v /unxmacxp.pro` ; do mkdir -p "$$i" ; done'
	cd "$<" ; sh -e -c 'for i in `cd "$(PWD)/$(BUILD_HOME)/$<" ; find . ! -type d | grep -v /CVS/ | grep -v /unxmacxp.pro` ; do if [ ! -f "$$i" ] ; then ln -sf "$(PWD)/$(BUILD_HOME)/$</$$i" "$$i" 2>/dev/null ; fi ; done'
	sh -e -c 'if [ ! -d "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro.oo" -a -d "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro" ] ; then rm -Rf "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro.oo" ; mv -f "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro" "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro.oo" ; fi'
	rm -Rf "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro"
	mkdir -p "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro"
	cd "$<" ; ln -sf "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro"
	source "$(OO_ENV_JAVA)" ; cd "$<" ; `alias build` $(NEO_BUILD_ARGS)
	touch "$@"

build.neo_patches: build.oo_all \
	build.neo_desktop_patch \
	build.neo_dtrans_patch \
	build.neo_forms_patch \
	build.neo_readlicense_patch \
	build.neo_offmgr_patch \
	build.neo_sal_patch \
	build.neo_setup2_patch \
	build.neo_sfx2_patch \
	build.neo_svtools_patch \
	build.neo_sysui_patch \
	build.neo_toolkit_patch \
	build.neo_vcl_patch \
	build.neo_instsetoo_patch
	touch "$@"

build.neo_odk_patches: \
	build.oo_odk_all \
	build.neo_odk_patch
	touch "$@"

build.oo_download_dics:
	mkdir -p "$(DIC_HOME)"
	rm -f "$(DIC_HOME)/dictionary.lst"
	curl -L "$(OO_DIC_URL)/available.lst" | awk -F, '{ print "DICT " $$1 " " $$2 " " $$3 }' >> "$(DIC_HOME)/dictionary.lst"
	curl -L "$(OO_DIC_URL)/hyphavail.lst" | awk -F, '{ print "HYPH " $$1 " " $$2 " " $$3 }' >> "$(DIC_HOME)/dictionary.lst"
	curl -L "$(OO_DIC_URL)/thesavail.lst" | awk -F, '{ print "THES " $$1 " " $$2 " " $$3 }' >> "$(DIC_HOME)/dictionary.lst"
	cd "$(DIC_HOME)" ; sh -e -c 'for i in `curl -L -l "$(OO_DIC_URL)" | grep "\.zip<" | sed "s#<\/A>.*\\$$##" | sed "s#^.*>##"` ; do curl -L -O "$(OO_DIC_URL)/$$i" ; done'
	touch "$@"

build.package: build.neo_patches build.oo_download_dics build.source_zip
	sh -e -c 'if [ -d "$(INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(INSTALL_HOME)" ; fi'
	mkdir -p "$(INSTALL_HOME)/package"
	echo `source "$(OO_ENV_JAVA)" ; cd "instsetoo/util" ; dmake language_numbers` > "$(INSTALL_HOME)/language_numbers"
	echo `source "$(OO_ENV_JAVA)" ; cd "instsetoo/util" ; dmake language_names` > "$(INSTALL_HOME)/language_names"
	echo "[ENVIRONMENT]" > "$(INSTALL_HOME)/response"
	echo "INSTALLATIONMODE=INSTALL_NETWORK" >> "$(INSTALL_HOME)/response"
	echo "INSTALLATIONTYPE=STANDARD" >> "$(INSTALL_HOME)/response"
	echo "DESTINATIONPATH=$(PWD)/$(INSTALL_HOME)/package/Contents" >> "$(INSTALL_HOME)/response"
	echo "OUTERPATH=" >> "$(INSTALL_HOME)/response"
	echo "LOGFILE=" >> "$(INSTALL_HOME)/response"
	echo "LANGUAGELIST="`cat "$(INSTALL_HOME)/language_numbers"` >> "$(INSTALL_HOME)/response"
	echo "[JAVA]" >> "$(INSTALL_HOME)/response"
	echo "JavaSupport=preinstalled_or_none" >> "$(INSTALL_HOME)/response"
# Eliminate duplicate help directories since only English is available
	mkdir -p "$(INSTALL_HOME)/package/Contents/help/en"
	cd "$(INSTALL_HOME)/package/Contents/help" ; sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names"` ; do ln -s "en" "$${i}" ; done'
	source "$(OO_ENV_JAVA)" ; "$(BUILD_HOME)/instsetoo/unxmacxp.pro/"`cat "$(INSTALL_HOME)/language_numbers"`"/normal/setup" -v "-r:$(PWD)/$(INSTALL_HOME)/response"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/dtrans/unxmacxp.pro/lib/libdtransjava$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/forms/unxmacxp.pro/lib/libfrm$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/sal/unxmacxp.pro/lib/libsal.dylib.3.0.1" "$(PWD)/$(BUILD_HOME)/sal/unxmacxp.pro/lib/libsalextra_x11osx_mxp.dylib" "$(PWD)/$(BUILD_HOME)/sfx2/unxmacxp.pro/lib/libsfx$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/svtools/unxmacxp.pro/lib/libsvt$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/toolkit/unxmacxp.pro/lib/libtk$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/vcl/unxmacxp.pro/lib/libvcl$${UPD}$${DLLSUFFIX}.dylib" "program"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/readlicense/source/license/unx/LICENSE" "$(PWD)/readlicense/source/readme/unxmacxp/README" "."
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_numbers" | sed "s#,# #g"` ; do cp "$(PWD)/$(BUILD_HOME)/offmgr/unxmacxp.pro/bin/neojava$${UPD}$${i}.res" "program/resource/iso$${UPD}$${i}.res" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/desktop/unxmacxp.pro/bin/soffice" "program/soffice.bin" ; chmod a+x "program/soffice.bin"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(LANGUAGE_NAMES)#'"`cat "$(PWD)/$(INSTALL_HOME)/language_names"`"'#g' "$(PWD)/$(BUILD_HOME)/setup2/unxmacxp.pro/misc/setup.sh"  > "program/setup" ; chmod a+x "program/setup"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/sysui/unxmacxp.pro/misc/nswrapper.sh" "program/nswrapper" ; chmod a+x "program/nswrapper"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/sysui/unxmacxp.pro/misc/Info.plist" "."
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/sysui/unxmacxp.pro/misc/PkgInfo" "."
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/unxmacxp.pro/bin/salapp"*.res "program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/unxmacxp.pro/class/vcl.jar" "program/classes"
	rm -Rf "$(INSTALL_HOME)/package/Contents/Resources"
	mkdir -p "$(INSTALL_HOME)/package/Contents/Resources"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/sysui/unxmacxp.pro/misc/License" "$(PWD)/$(BUILD_HOME)/sysui/unxmacxp.pro/misc/Readme" "$(PWD)/$(BUILD_HOME)/sysui/unxmacxp.pro/misc/"*.icns "Resources"
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names" | sed "s#-#_#g"` ; do mkdir -p "$${i}.lproj" ; mkdir -p `echo "$${i}" | sed "s#_.*\\$$##"`".lproj" ;  done'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/program" ; regcomp -revoke -r applicat.rdb -c "libdtransX11$${UPD}$${DLLSUFFIX}.dylib"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/program" ; regcomp -register -r applicat.rdb -c "libdtransjava$${UPD}$${DLLSUFFIX}.dylib"
	cd "$(INSTALL_HOME)/package/Contents/program" ; ln -s "libjava_uno.dylib" "libjava_uno.jnilib" ; ln -s "libdb_java-3.2.dylib" "libdb_java-3.2.jnilib"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "license.html" "readme.html" "spadmin" "setup" "program/libdtransX11$${UPD}$${DLLSUFFIX}.dylib" "program/libpsp$${UPD}$${DLLSUFFIX}.dylib" "program/libspa$${UPD}$${DLLSUFFIX}.dylib" "program/setup.bin" "program/soffice" "program/spadmin" "program/spadmin.bin" "share/kde" "share/psprint"
	cd "$(INSTALL_HOME)/package/Contents/program" ; ln -s "soffice.bin" "soffice"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#ProductPatch=.*$$#ProductPatch=($(PRODUCT_VERSION))#' "program/bootstraprc" | sed '/Location=.*$$/d' | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/Library/$(PRODUCT_DIR_NAME)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' > "../../../out" ; mv -f "../../../out" "program/bootstraprc"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "share/config/registry/cache" ; sh -e -c 'for i in "share/config/registry/instance/org/openoffice/Setup.xml" "share/config/registry/instance/org/openoffice/Office/Common.xml" ; do sed "s#\"string\">.*</ooName>#\"string\">$(PRODUCT_NAME)</ooName>#g" "$${i}" | sed "s#\"string\">.*</ooSetupVersion>#\"string\">$(PRODUCT_VERSION)</ooSetupVersion>#g" | sed "s#$(PWD)/$(INSTALL_HOME)/package/Contents#\$$\(insturl\)#g" | sed "s#>OpenOffice\.org [0-9\.]* #>$(PRODUCT_NAME) $(PRODUCT_VERSION) #g" | sed "s#/work#/../../../Documents#g" | sed "s#>UNIX<#>MAC<#g" > "../../../out" ; mv -f "../../../out" "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*" -o -name "*.bin"` ; do strip -S -x "$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'if [ ! -d "MacOS" ] ; then rm -Rf "MacOS" ; mv -f "program" "MacOS" ; ln -s "MacOS" "program" ; fi'
	cd "$(INSTALL_HOME)/package/Contents/share/dict/ooo" ; sh -c 'for i in `sed "s#-[a-zA-Z0-9]* # #g" "$(PWD)/$(INSTALL_HOME)/language_names"` ; do for j in "$(PWD)/$(DIC_HOME)"/*.zip ; do unzip -o "$$j" "$$i*.aff" "$$i*.dic" "hyph_$$i*.dic" "th_$$i*.dat" "th_$$i*.idx" ; if [ $$? != 0 -a $$? != 11 ] ; then exit $$? ; fi ; done ; done'
	cd "$(INSTALL_HOME)/package/Contents/share/dict/ooo" ; rm -f "dictionary.lst" ; sh -c 'for i in `ls -1 *.dic *.dat | sort -u | sed "s#\\.dic\\$$##" | sed "s#\\.dat\\$$##"`; do grep " $$i\$$" "$(PWD)/$(DIC_HOME)/dictionary.lst" >> "dictionary.lst" ; if [ $$? != 0 -a $$? != 1 ] ; then exit $$? ; fi ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find user -type d | grep -v /CVS$$` ; do mkdir -p "$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find user ! -type d | grep -v /CVS/` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find "." -name ".DS_Store"` ; do rm "$${i}" ; done'
	chmod -Rf a-w,a+r "$(INSTALL_HOME)/package"
	echo "Running sudo to chown installation files..."
	sudo chown -Rf root:admin "$(INSTALL_HOME)/package"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/neojava.info" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).info"
	/usr/bin/package "$(INSTALL_HOME)/package" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).info" -d "$(INSTALL_HOME)"
	cp "etc/gpl.html" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/License.html"
	cp "bin/InstallationCheck" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck"
	cp "bin/preflight" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight"
# Mac OS X 10.2.8 cannot handle a postflight script
	cp "bin/postflight" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/$(PRODUCT_DIR_NAME).post_install" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/$(PRODUCT_DIR_NAME).post_install"
	cp "bin/postflight" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/$(PRODUCT_DIR_NAME).post_upgrade" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/$(PRODUCT_DIR_NAME).post_upgrade"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)"
	mv -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)/src"
	cp -f "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION).src.tar.gz" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)/src"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Install.html" | sed 's#$$(PRODUCT_INSTALL_URL)#$(PRODUCT_INSTALL_URL)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)/Install.html"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Build.html" | sed 's#$$(PRODUCT_BUILD_URL)#$(PRODUCT_BUILD_URL)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)/src/Build.html"
	chmod -Rf a-w,a+r "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)"
	touch "$@"

build.odk_package: build.neo_odk_patches
	touch "$@"

build.source_zip:
	rm -Rf "$(SOURCE_HOME)"
	mkdir -p "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)"
	cd "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)" ; cvs -d "$(NEO_CVSROOT)" co -r "$(NEO_TAG)" "$(NEO_PACKAGE)"
# Prune out empty directories
	cd "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)" ; sh -e -c 'for i in `ls -1`; do cd "$${i}" ; cvs update -P -d ; done'
	cp "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)/neojava/etc/gpl.html" "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)/LICENSE.html"
	chmod -Rf u+w,og-w,a+r "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)"
	cd "$(SOURCE_HOME)" ; gnutar zcf "$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION).src.tar.gz" "$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION)"
	touch "$@"

build.all: build.package
	touch "$@"
