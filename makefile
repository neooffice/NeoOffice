########################################################################
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
#   Copyright 2003 Planamesa Inc.
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

# Macros that are overridable by make command line options
ifeq ("$(shell uname -s)","Darwin")
JDK_HOME=/System/Library/Frameworks/JavaVM.framework/Home
else
# JDK_HOME must be defined in custom.mk
endif
PRODUCT_NAME=My Untested Office Suite
PRODUCT_DIR_NAME=$(subst $(SPACE),,$(PRODUCT_NAME))
PRODUCT_DIR_NAME2=$(PRODUCT_DIR_NAME)
PRODUCT_DIR_NAME3=$(PRODUCT_DIR_NAME)
PRODUCT_DOMAIN=com.mycompany
PRODUCT_TRADEMARKED_NAME=$(PRODUCT_NAME)
PRODUCT_TRADEMARKED_NAME_RTF=$(PRODUCT_NAME)
PRODUCT_DIR_PATCH_VERSION_EXTRA=

# Custom overrides go in the following file
-include custom.mk

# Custom code signing certificate macros go in the following file
-include certs.mk

EMPTY:=
SPACE:=$(EMPTY) $(EMPTY)
ifndef TMP
TMP:=/tmp
endif
# Set the shell to bash to match the LibreOffice build
SHELL:=/bin/bash
UNAME:=$(shell uname -p)
ifeq ("$(shell uname -s)","Darwin")
OS_TYPE=macOS
OS_MAJOR_VERSION:=$(shell /usr/bin/sw_vers | grep '^ProductVersion:' | awk '{ print $$2 }' | awk -F. '{ print $$1 "." $$2 }')
ULONGNAME=Intel
CPUNAME=I
UOUTPUTDIR=unxmaccx.pro
DLLSUFFIX=mxi
TARGET_MACHINE=x86_64
TARGET_FILE_TYPE=Mach-O 64-bit executable $(TARGET_MACHINE)
else
OS_TYPE=Win32
UOUTPUTDIR=wntmsci12.pro
DLLSUFFIX=mxp
endif
BUILD_MACHINE=$(shell echo `id -nu`:`hostname`.`domainname`)

# Build location macros
BUILD_HOME:=build
ifdef PRODUCT_BUILD3
INSTALL_HOME:=install3
PATCH_INSTALL_HOME:=patch_install3
else ifdef PRODUCT_BUILD2
INSTALL_HOME:=install2
PATCH_INSTALL_HOME:=patch_install2
else
INSTALL_HOME:=install
PATCH_INSTALL_HOME:=patch_install
endif
SOURCE_HOME:=source
CD_INSTALL_HOME:=cd_install
APACHE_PATCHES_HOME:=patches/apache
NEOOFFICE_PATCHES_HOME:=patches/neooffice
LIBO_PATCHES_HOME:=patches/libreoffice
LIBO_PACKAGE=libreoffice-4.4.7.2
LIBO_SOURCE_FILE=$(LIBO_PACKAGE).tar.xz
LIBO_BUILD_HOME=$(BUILD_HOME)/$(LIBO_PACKAGE)
LIBO_OVERRIDDEN_ENVVARS:=$(BUILD_HOME)/config_host.mk
LIBO_LANGUAGES:=$(shell cat '$(PWD)/etc/supportedlanguages.txt' | sed '/^\#.*$$/d' | sed 's/\#.*$$//' | awk -F, '{ print $$1 }')
NEOLIGHT_MDIMPORTER_ID:=org.neooffice.neolight
NEOPEEK_QLPLUGIN_ID:=org.neooffice.quicklookplugin

# Product information
OO_PRODUCT_VERSION_FAMILY=4.1
OO_PRODUCT_NAME=Apache OpenOffice
OO_PRODUCT_VERSION=$(OO_PRODUCT_VERSION_FAMILY).3
PRODUCT_INSTALL_DIR_NAME=$(PRODUCT_NAME)
PRODUCT_VERSION_FAMILY=4.0
PRODUCT_VERSION_BASE=2016
PRODUCT_VERSION=$(PRODUCT_VERSION_BASE)
PRODUCT_VERSION2=$(PRODUCT_VERSION) $(PRODUCT_VERSION_EXT2)
PRODUCT_VERSION3=$(PRODUCT_VERSION) $(PRODUCT_VERSION_EXT3)
PRODUCT_VERSION_EXT=Beta
PRODUCT_VERSION_EXT2=Viewer Beta
PRODUCT_VERSION_EXT3=Classic Edition Beta
PRODUCT_DIR_VERSION=$(subst $(SPACE),_,$(PRODUCT_VERSION))
PRODUCT_SHORT_VERSION=$(subst $(SPACE),,$(subst $(PRODUCT_VERSION_EXT),,$(PRODUCT_VERSION)))
PREVIOUS_PRODUCT_VERSION_BASE=$(PRODUCT_VERSION_BASE)
PREVIOUS_PRODUCT_VERSION=$(PRODUCT_VERSION)
PRODUCT_LANG_PACK_VERSION=Language Pack
PRODUCT_DIR_LANG_PACK_VERSION=$(subst $(SPACE),_,$(PRODUCT_LANG_PACK_VERSION))
PRODUCT_PATCH_VERSION=Patch 0
PRODUCT_DIR_PATCH_VERSION=$(subst $(SPACE),-,$(PRODUCT_PATCH_VERSION))
ifdef PRODUCT_DIR_PATCH_VERSION_EXTRA
ifeq ($(PRODUCT_DIR_PATCH_VERSION),)
PRODUCT_DIR_PATCH_VERSION:=$(PRODUCT_DIR_PATCH_VERSION_EXTRA)
else
PRODUCT_DIR_PATCH_VERSION:=$(PRODUCT_DIR_PATCH_VERSION)-$(PRODUCT_DIR_PATCH_VERSION_EXTRA)
endif
endif
PRODUCT_MIN_OSVERSION=10.10
PRODUCT_MIN_OSVERSION_NAME=$(PRODUCT_MIN_OSVERSION) Yosemite
PRODUCT_MAX_OSVERSION=10.12
PRODUCT_MAX_OSVERSION_NAME=$(PRODUCT_MAX_OSVERSION) Sierra
PRODUCT_FILETYPE=NO%F
PRODUCT_BASE_URL=http://www.neooffice.org/neojava
PRODUCT_SUPPORT_URL=$(PRODUCT_BASE_URL)/contact.php
PRODUCT_SUPPORT_URL_TEXT:=$(PRODUCT_NAME) Support
PRODUCT_DOCUMENTATION_URL=http://neowiki.neooffice.org/
PRODUCT_DOCUMENTATION_URL_TEXT=$(PRODUCT_NAME) Wiki
PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL=http://neowiki.neooffice.org/index.php/NeoOffice_Launch_Shortcuts
PRODUCT_DOCUMENTATION_SPELLCHECK_URL=http://neowiki.neooffice.org/index.php/Activating_Dictionaries_and_Configuring_Spellcheck
PRODUCT_UPDATE_CHECK_URL=$(PRODUCT_BASE_URL)/patchcheck.php
PRODUCT_MAC_APP_STORE_URL=macappstores://itunes.apple.com/app/neooffice/id639210716?mt=12
PRODUCT_DOWNLOAD_URL=$(PRODUCT_BASE_URL)/downloadfromproduct.php
PRODUCT_JAVA_DOWNLOAD_URL=$(PRODUCT_BASE_URL)/javadownload.php
PRODUCT_BUNDLED_LANG_PACKS=en-US de fr it he ja ar es ru nl en-GB sv pl nb fi pt-BR da zh-TW cs th zh-CN el hu sk ko tr
PRODUCT_BUNDLED_LANG_PACKS2=$(PRODUCT_BUNDLED_LANG_PACKS)
PRODUCT_BUNDLED_LANG_PACKS3=$(PRODUCT_BUNDLED_LANG_PACKS)
ifeq ("$(OS_TYPE)","macOS")
PRODUCT_COMPONENT_MODULES=
PRODUCT_COMPONENT_PATCH_MODULES=
PREFLIGHT_REQUIRED_COMMANDS=defaults find id open touch
INSTALLATION_CHECK_REQUIRED_COMMANDS=$(PREFLIGHT_REQUIRED_COMMANDS) awk basename chmod dirname file grep mv pax rm ps pwd sed sort unzip
ifdef PRODUCT_BUILD3
PRODUCT_USER_INSTALL_DIR=$$HOME/Library/Preferences/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)
else
PRODUCT_USER_INSTALL_DIR=$$HOME/Library/Containers/$(PRODUCT_DOMAIN).$(PRODUCT_DIR_NAME)/Data/Library/Preferences/$(PRODUCT_DIR_NAME)
endif
else
PRODUCT_COMPONENT_MODULES=
PRODUCT_COMPONENT_PATCH_MODULES=
endif

# CVS macros
ANT_PACKAGE=apache-ant-1.9.6
ANT_SOURCE_FILENAME=apache-ant-1.9.6-bin.tar.gz
YOURSWAYCREATEDMG_PACKAGE=jaeggir-yoursway-create-dmg-a22ac11
YOURSWAYCREATEDMG_SOURCE_FILENAME=yoursway-create-dmg.zip
NEO_CVSROOT:=:pserver:anoncvs@anoncvs.neooffice.org:/cvs
NEO_PACKAGE:=NeoOffice
NEO_TAG:=NeoOffice-2016_Beta
NEO_TAG2:=NeoOffice-2016_Viewer_Beta
NEO_TAG3:=NeoOffice-2016_Classic_Edition_Beta

all: build.all

# Include dependent makefiles
include neo_configure.mk

build.libo_src_checkout: $(LIBO_PATCHES_HOME)/$(LIBO_SOURCE_FILE)
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; tar zxvf "$(PWD)/$<"
	cd "$(BUILD_HOME)" ; chmod -Rf u+rw "$(LIBO_PACKAGE)"
	cd "$(BUILD_HOME)" ; chmod a+x "$(LIBO_PACKAGE)/bin/unpack-sources"
	touch "$@"

build.ant_checkout: $(APACHE_PATCHES_HOME)/$(ANT_SOURCE_FILENAME)
	rm -Rf "$(BUILD_HOME)/$(ANT_PACKAGE)"
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; tar zxvf "$(PWD)/$(APACHE_PATCHES_HOME)/$(ANT_SOURCE_FILENAME)"
	touch "$@"

build.libo_external_tarballs_checkout: build.libo_src_checkout
	rm -Rf "$(LIBO_BUILD_HOME)/external/tarballs"
	mkdir -p "$(LIBO_BUILD_HOME)/external/tarballs"
	cd "$(LIBO_PATCHES_HOME)/external/tarballs" ; sh -c -e 'for i in `find . -type f -maxdepth 1 | grep -v /CVS/` ; do cp "$$i" "$(PWD)/$(LIBO_BUILD_HOME)/external/tarballs/$$i" ; done'
	touch "$@"

build.libo_checkout: build.libo_src_checkout build.ant_checkout build.libo_external_tarballs_checkout
	touch "$@"

build.libo_patches: \
	build.libo_bin_patch \
	build.libo_solenv_patch \
	build.libo_sw_patch
	touch "$@"

build.libo_%_patch: $(LIBO_PATCHES_HOME)/%.patch build.libo_checkout
ifeq ("$(OS_TYPE)","macOS")
	-( cd "$(LIBO_BUILD_HOME)/$(@:build.libo_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(LIBO_BUILD_HOME)/$(@:build.libo_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
else
	-cat "$<" | unix2dos | ( cd "$(LIBO_BUILD_HOME)/$(@:build.libo_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" )
	cat "$<" | unix2dos | ( cd "$(LIBO_BUILD_HOME)/$(@:build.libo_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" )
endif
	touch "$@"

build.libo_configure: build.libo_patches
ifeq ("$(OS_TYPE)","macOS")
	cd "$(LIBO_BUILD_HOME)" ; PATH="/bin:/sbin:/usr/bin:/usr/sbin:/opt/local/bin" ; export PATH ; unset DYLD_LIBRARY_PATH ; autoconf ; ./configure --without-parallelism --with-jdk-home="$(JDK_HOME)" --with-ant-home="$(PWD)/$(BUILD_HOME)/$(ANT_PACKAGE)" --with-macosx-version-min-required="$(PRODUCT_MIN_OSVERSION)" --without-junit --disable-cups --disable-odk --with-lang="$(LIBO_LANGUAGES)" --without-fonts
else
ifndef JDK_HOME
	@echo "JDK_HOME must be defined in custom.mk" ; exit 1
endif
	@sh -c -e 'if [ ! -r "$(JDK_HOME)/lib/tools.jar" ] ; then echo "You must set JDK_HOME to the path of a valid Java Development Kit" ; exit 1 ; fi'
	@echo "$@ not implemented" ; exit 1
endif
	touch "$@"

build.libo_all: build.libo_configure
ifeq ("$(OS_TYPE)","macOS")
	cd "$(LIBO_BUILD_HOME)" ; make
else
	@echo "$@ not implemented" ; exit 1
endif
	touch "$@"

build.neo_configure: build.libo_all neo_configure.mk
	$(MAKE) $(MFLAGS) build.neo_configure_phony
	touch "$@"

build.neo_patches: \
	$(PRODUCT_COMPONENT_MODULES:%=build.neo_%_component) \
	$(PRODUCT_COMPONENT_PATCH_MODULES:%=build.neo_%_component) \
	build.neo_avmedia_patch \
	build.neo_basic_patch \
	build.neo_canvas_patch \
	build.neo_connectivity_patch \
	build.neo_cppcanvas_patch \
	build.neo_cppuhelper_patch \
	build.neo_cui_patch \
	build.neo_dbaccess_patch \
	build.neo_desktop_patch \
	build.neo_drawinglayer_patch \
	build.neo_editeng_patch \
	build.neo_extensions_patch \
	build.neo_filter_patch \
	build.neo_fpicker_patch \
	build.neo_framework_patch \
	build.neo_hsqldb_patch \
	build.neo_i18npool_patch \
	build.neo_jvmfwk_patch \
	build.neo_lingucomponent_patch \
	build.neo_linguistic_patch \
	build.neo_oox_patch \
	build.neo_package_patch \
	build.neo_pyuno_patch \
	build.neo_reportdesign_patch \
	build.neo_sal_patch \
	build.neo_sc_patch \
	build.neo_sd_patch \
	build.neo_sdext_patch \
	build.neo_sfx2_patch \
	build.neo_shell_patch \
	build.neo_svl_patch \
	build.neo_svtools_patch \
	build.neo_svx_patch \
	build.neo_sw_patch \
	build.neo_tools_patch \
	build.neo_vcl_patch \
	build.neo_ucb_patch \
	build.neo_ucbhelper_patch \
	build.neo_unotools_patch \
	build.neo_unoxml_patch \
	build.neo_uui_patch \
	build.neo_vos_patch \
	build.neo_writerfilter_patch \
	build.neo_xmloff_patch
	touch "$@"

# Custom modules that need to link directly to other custom modules
build.neo_remotecontrol_component: build.remotecontrol_patches
build.neo_sc_patch: build.neo_unotools_patch
build.neo_sfx2_patch: build.neo_sal_patch
build.neo_sw_patch: build.neo_unotools_patch

build.neo_%_patch: % build.neo_configure
	rm -Rf "$(PWD)/$</$(UOUTPUTDIR)"
	cd "$<" ; sh -e -c '( cd "$(PWD)/$(OO_BUILD_HOME)/$<" ; find . -type d | sed "s/ /\\ /g" | grep -v /CVS$$ | grep -v /$(UOUTPUTDIR) ) | while read i ; do mkdir -p "$$i" ; done'
ifeq ("$(OS_TYPE)","macOS")
	cd "$<" ; sh -e -c '( cd "$(PWD)/$(OO_BUILD_HOME)/$<" ; find . ! -type d | sed "s/ /\\ /g" | grep -v /CVS/ | grep -v /$(UOUTPUTDIR) ) | while read i ; do if [ ! -f "$$i" ] ; then ln -sf "$(PWD)/$(OO_BUILD_HOME)/$</$$i" "$$i" 2>/dev/null ; fi ; done'
else
# Use hardlinks for Windows
	cd "$<" ; sh -e -c 'CYGWIN=winsymlinks ; export CYGWIN ; ( cd "$(PWD)/$(OO_BUILD_HOME)/$<" ; find . ! -type d | grep -v /CVS/ | grep -v /$(UOUTPUTDIR) ) | while read i ; do if [ ! -f "$$i" ] ; then ln -f "$(PWD)/$(OO_BUILD_HOME)/$</$$i" "$$i" 2>/dev/null ; fi ; done'
endif
	source "$(OO_ENV_JAVA)" ; cd "$<" ; `alias build` $(NEO_BUILD_ARGS)
	touch "$@"

build.neo_%_component: % build.neo_configure
	rm -Rf "$(PWD)/$</$(UOUTPUTDIR)"
	mkdir -p "$(PWD)/$</$(UOUTPUTDIR)"
	source "$(OO_ENV_JAVA)" ; cd "$<" ; `alias build` $(NEO_BUILD_ARGS)
	touch "$@"

build.package: build.neo_patches
ifeq ("$(PRODUCT_PATCH_VERSION)","Patch 0")
	"$(MAKE)" $(MFLAGS) PRODUCT_PATCH_VERSION=" " "build.package"
else
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" -o "$$PRODUCT_DOMAIN" != "$(PRODUCT_DOMAIN)" -o "$$PRODUCT_DIR_NAME" != "$(PRODUCT_DIR_NAME)" -o "$$PRODUCT_MAC_APP_STORE_URL" != "$(PRODUCT_MAC_APP_STORE_URL)" -o "$$PRODUCT_JAVA_DOWNLOAD_URL" != "$(PRODUCT_JAVA_DOWNLOAD_URL)" -o "$$PRODUCT_MIN_OSVERSION" != "$(PRODUCT_MIN_OSVERSION)" -o "$$PRODUCT_MAX_OSVERSION" != "$(PRODUCT_MAX_OSVERSION)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	"$(MAKE)" $(MFLAGS) "build.package_shared"
	touch "$@"
endif

build.package2: build.neo_patches
ifndef PRODUCT_BUILD2
ifeq ("$(PRODUCT_PATCH_VERSION)","Patch 0")
	"$(MAKE)" $(MFLAGS) PRODUCT_PATCH_VERSION=" " "build.package2"
else
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" -o "$$PRODUCT_DOMAIN" != "$(PRODUCT_DOMAIN)" -o "$$PRODUCT_DIR_NAME2" != "$(PRODUCT_DIR_NAME2)" -o "$$PRODUCT_MAC_APP_STORE_URL" != "$(PRODUCT_MAC_APP_STORE_URL)" -o "$$PRODUCT_JAVA_DOWNLOAD_URL" != "$(PRODUCT_JAVA_DOWNLOAD_URL)" -o "$$PRODUCT_MIN_OSVERSION" != "$(PRODUCT_MIN_OSVERSION)" -o "$$PRODUCT_MAX_OSVERSION" != "$(PRODUCT_MAX_OSVERSION)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	"$(MAKE)" $(MFLAGS) "PRODUCT_BUILD2=TRUE" "PRODUCT_VERSION=$(PRODUCT_VERSION2)" "PRODUCT_VERSION_EXT=$(PRODUCT_VERSION_EXT2)" "PRODUCT_DIR_NAME=$(PRODUCT_DIR_NAME2)" "CERTAPPIDENTITY=$(CERTAPPIDENTITY2)" "CERTPKGIDENTITY=$(CERTPKGIDENTITY2)" "PRODUCT_BUNDLED_LANG_PACKS=$(PRODUCT_BUNDLED_LANG_PACKS2)" "build.package_shared"
	touch "$@"
endif
endif

build.package3: build.neo_patches
ifndef PRODUCT_BUILD3
ifeq ("$(PRODUCT_PATCH_VERSION)","Patch 0")
	"$(MAKE)" $(MFLAGS) PRODUCT_PATCH_VERSION=" " "build.package3"
else
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" -o "$$PRODUCT_DOMAIN" != "$(PRODUCT_DOMAIN)" -o "$$PRODUCT_DIR_NAME3" != "$(PRODUCT_DIR_NAME3)" -o "$$PRODUCT_MAC_APP_STORE_URL" != "$(PRODUCT_MAC_APP_STORE_URL)" -o "$$PRODUCT_JAVA_DOWNLOAD_URL" != "$(PRODUCT_JAVA_DOWNLOAD_URL)" -o "$$PRODUCT_MIN_OSVERSION" != "$(PRODUCT_MIN_OSVERSION)" -o "$$PRODUCT_MAX_OSVERSION" != "$(PRODUCT_MAX_OSVERSION)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	"$(MAKE)" $(MFLAGS) "PRODUCT_BUILD3=TRUE" "PRODUCT_VERSION=$(PRODUCT_VERSION3)" "PRODUCT_VERSION_EXT=$(PRODUCT_VERSION_EXT3)" "PRODUCT_DIR_NAME=$(PRODUCT_DIR_NAME3)" "CERTAPPIDENTITY=$(CERTAPPIDENTITY3)" "CERTPKGIDENTITY=$(CERTPKGIDENTITY3)" "PRODUCT_BUNDLED_LANG_PACKS=$(PRODUCT_BUNDLED_LANG_PACKS3)" "build.package_shared"
	touch "$@"
endif
endif

build.package_shared:
# Check that codesign and productsign executables exist before proceeding
	@sh -e -c 'for i in codesign productsign ; do if [ -z "`which $$i`" ] ; then echo "$$i command not found" ; exit 1 ; fi ; done'
	sh -e -c 'if [ -d "$(INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(INSTALL_HOME)" ; fi'
	sh -e -c 'if [ -d "/Volumes/OpenOffice" ] ; then hdiutil eject -force "/Volumes/OpenOffice" ; fi'
	hdiutil attach -nobrowse "$(OO_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/$(subst $(SPACE),_,$(OO_PRODUCT_NAME))/dmg/install/en-US/$(subst $(SPACE),_,$(OO_PRODUCT_NAME))_$(OO_PRODUCT_VERSION)_MacOS_$(subst _,-,$(TARGET_MACHINE))_install_en-US.dmg"
	mkdir -p "$(INSTALL_HOME)/package/Contents"
	cd "$(INSTALL_HOME)/package" ; ( ( cd "/Volumes/OpenOffice/OpenOffice.app" ; gnutar cvf - . ) | ( cd "$(PWD)/$(INSTALL_HOME)/package" ; gnutar xvf - --exclude="._*" ) )
	hdiutil eject -force "/Volumes/OpenOffice"
# Regroup the OOo language packs
	cd "$(OO_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/$(subst $(SPACE),_,$(OO_PRODUCT_NAME))_languagepack/dmg/install" ; find . -type d -maxdepth 1 -exec basename {} \; | grep -v '^\.$$' | grep -v '^follow_me$$' | grep -v '^log$$' > "$(PWD)/$(INSTALL_HOME)/language_names"
# Include certain languages in the main installer
	sh -e -c 'for i in $(PRODUCT_BUNDLED_LANG_PACKS) ; do if [ -d "/Volumes/OpenOffice Language Pack" ] ; then hdiutil eject -force "/Volumes/OpenOffice Language Pack" ; fi ; hdiutil attach -nobrowse "$(PWD)/$(OO_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/$(subst $(SPACE),_,$(OO_PRODUCT_NAME))_languagepack/dmg/install/$${i}/$(subst $(SPACE),_,$(OO_PRODUCT_NAME))_$(OO_PRODUCT_VERSION)_MacOS_$(subst _,-,$(TARGET_MACHINE))_langpack_$${i}.dmg" ; bunzip2 -dc "/Volumes/OpenOffice Language Pack/OpenOffice Language Pack.app/Contents/tarball.tar.bz2" | ( cd "$(PWD)/$(INSTALL_HOME)/package" ; gnutar xvf - --exclude="._*" ) ; hdiutil eject -force "/Volumes/OpenOffice Language Pack" ; helpflag=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | awk -F, "{ print \\$$2 }"` ; if [ "$${helpflag}" != "1" ] ; then rm -Rf "$(PWD)/$(INSTALL_HOME)/package/Contents/help/$${i}" ; ( cd "$(PWD)/$(INSTALL_HOME)/package/Contents/help" ; ln -s "en" "$${i}" ) ; fi ; done'
ifndef LANGPACKS
# Bypass the language pack installers
else
# Create the language pack installers
	sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names" | sed $(foreach BUNDLED_LANG_PACK,$(PRODUCT_BUNDLED_LANG_PACKS),-e "/^$(BUNDLED_LANG_PACK)\\$$/d")` ; do langname=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | sed "s/#.*$$//" | awk -F, "{ print \\$$3 }"` ; langdirname=`echo "$${langname}" | sed "s# #_#g"` ; if [ -z "$${langname}" -o -z "$${langdirname}" ] ; then echo "Skipping $${i} language..." ; continue ; fi ; mkdir -p "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents" ; if [ -d "/Volumes/OpenOffice Language Pack" ] ; then hdiutil eject -force "/Volumes/OpenOffice Language Pack" ; fi ; hdiutil attach -nobrowse "$(PWD)/$(OO_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/$(subst $(SPACE),_,$(OO_PRODUCT_NAME))_languagepack/dmg/install/$${i}/$(subst $(SPACE),_,$(OO_PRODUCT_NAME))_$(OO_PRODUCT_VERSION)_MacOS_$(subst _,-,$(TARGET_MACHINE))_langpack_$${i}.dmg" ; bunzip2 -dc "/Volumes/OpenOffice Language Pack/OpenOffice Language Pack.app/Contents/tarball.tar.bz2" | ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}" ; gnutar xvf - --exclude="._*" ) ; hdiutil eject -force "/Volumes/OpenOffice Language Pack" ; helpflag=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | awk -F, "{ print \\$$2 }"` ; if [ "$${helpflag}" != "1" ] ; then rm -Rf "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help/$${i}" ; ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help" ; ln -s "en" "$${i}" ) ; fi ; "$(MAKE)" $(MFLAGS) "PRODUCT_LANG_PACK_LOCALE=$${i}" "PRODUCT_LANG_PACK_VERSION=$(PRODUCT_LANG_PACK_VERSION) $${langname}" "PRODUCT_DIR_LANG_PACK_VERSION=$(PRODUCT_DIR_LANG_PACK_VERSION)_$${langdirname}" "build.package_$${langdirname}" ; done'
endif
# Remove OOo system plugins but fix bug 3381 to save standard dictionaries
	rm -Rf "$(INSTALL_HOME)/package/Contents/Frameworks"
	rm -Rf "$(INSTALL_HOME)/package/Contents/Library"
	rm -Rf "$(INSTALL_HOME)/package/Contents/share/extension"
	rm -Rf "$(INSTALL_HOME)/package/Contents/share/uno_packages"
	mkdir -p "$(INSTALL_HOME)/package/Contents/share/extension"
	mkdir -p "$(INSTALL_HOME)/package/Contents/share/uno_packages"
	chmod -Rf u+w,a+r "$(INSTALL_HOME)/package"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program" ; mv -f "MacOS" "program" ; mkdir -p "MacOS"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/avmedia/$(UOUTPUTDIR)/lib/libavmedia.dylib" "$(PWD)/avmedia/$(UOUTPUTDIR)/lib/libavmediajava.dylib" "$(PWD)/basic/$(UOUTPUTDIR)/lib/libsb.dylib" "$(PWD)/canvas/$(UOUTPUTDIR)/lib/vclcanvas.uno.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libcalc.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libdbtools.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libmacab1.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libmacabdrv1.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libodbc.uno.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libodbcbase.dylib" "$(PWD)/cppcanvas/$(UOUTPUTDIR)/lib/libcppcanvas.dylib" "$(PWD)/cppuhelper/$(UOUTPUTDIR)/lib/libuno_cppuhelpers5abi.dylib.3" "$(PWD)/cui/$(UOUTPUTDIR)/lib/libcui.dylib" "$(PWD)/dbaccess/$(UOUTPUTDIR)/lib/libdba.dylib" "$(PWD)/dbaccess/$(UOUTPUTDIR)/lib/libdbu.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/deployment.uno.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libdeploymentgui.uno.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libdeploymentmisc.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libsofficeapp.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libspl.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libunopkgapp.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/migrationoo2.uno.dylib" "$(PWD)/drawinglayer/$(UOUTPUTDIR)/LinkTarget/Library/libdrawinglayer.dylib" "$(PWD)/editeng/$(UOUTPUTDIR)/LinkTarget/Library/libediteng.dylib" "$(PWD)/extensions/$(UOUTPUTDIR)/lib/libscn.dylib" "$(PWD)/filter/$(UOUTPUTDIR)/lib/libipt.dylib" "$(PWD)/filter/$(UOUTPUTDIR)/lib/libmsfilter.dylib" "$(PWD)/filter/$(UOUTPUTDIR)/lib/libpdffilter.dylib" "$(PWD)/fpicker/$(UOUTPUTDIR)/lib/fpicker.uno.dylib" "$(PWD)/fpicker/$(UOUTPUTDIR)/lib/fps_java.uno.dylib" "$(PWD)/framework/$(UOUTPUTDIR)/LinkTarget/Library/libfwe.dylib" "$(PWD)/framework/$(UOUTPUTDIR)/LinkTarget/Library/libfwk.dylib" "$(PWD)/i18npool/$(UOUTPUTDIR)/lib/i18npool.uno.dylib" "$(PWD)/jvmfwk/$(UOUTPUTDIR)/lib/libjvmfwk.dylib.3" "$(PWD)/lingucomponent/$(UOUTPUTDIR)/lib/libspell.uno.dylib" "$(PWD)/linguistic/$(UOUTPUTDIR)/lib/liblng.dylib" "$(PWD)/oox/$(UOUTPUTDIR)/lib/liboox.dylib" "$(PWD)/package/$(UOUTPUTDIR)/lib/libpackage2.dylib" "$(PWD)/reportdesign/$(UOUTPUTDIR)/lib/librpt.dylib" "$(PWD)/sal/$(UOUTPUTDIR)/lib/libuno_sal.dylib.3" "$(PWD)/sc/$(UOUTPUTDIR)/lib/libsc.dylib" "$(PWD)/sc/$(UOUTPUTDIR)/lib/libscui.dylib" "$(PWD)/sd/$(UOUTPUTDIR)/lib/libsd.dylib" "$(PWD)/sfx2/$(UOUTPUTDIR)/LinkTarget/Library/libsfx.dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/cmdmail.uno.dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/localebe1.uno.dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/macbe1.uno.dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/syssh.uno.dylib" "$(PWD)/svl/$(UOUTPUTDIR)/LinkTarget/Library/libsvl.dylib" "$(PWD)/svtools/$(UOUTPUTDIR)/LinkTarget/Library/libsvt.dylib" "$(PWD)/svx/$(UOUTPUTDIR)/LinkTarget/Library/libsvx.dylib" "$(PWD)/svx/$(UOUTPUTDIR)/LinkTarget/Library/libsvxcore.dylib" "$(PWD)/sw/$(UOUTPUTDIR)/LinkTarget/Library/libmsword.dylib" "$(PWD)/sw/$(UOUTPUTDIR)/LinkTarget/Library/libsw.dylib" "$(PWD)/sw/$(UOUTPUTDIR)/LinkTarget/Library/libswui.dylib" "$(PWD)/sw/$(UOUTPUTDIR)/LinkTarget/Library/vbaswobj.uno.dylib" "$(PWD)/tools/$(UOUTPUTDIR)/LinkTarget/Library/libtl.dylib" "$(PWD)/vcl/$(UOUTPUTDIR)/LinkTarget/Library/libvcl.dylib" "$(PWD)/ucb/$(UOUTPUTDIR)/lib/libucpdav1.dylib" "$(PWD)/ucb/$(UOUTPUTDIR)/lib/libucpfile1.dylib" "$(PWD)/ucbhelper/$(UOUTPUTDIR)/lib/libucbhelper4s5abi.dylib" "$(PWD)/unotools/$(UOUTPUTDIR)/lib/libutl.dylib" "$(PWD)/unoxml/$(UOUTPUTDIR)/LinkTarget/Library/libunoxml.dylib" "$(PWD)/uui/$(UOUTPUTDIR)/lib/libuui.dylib" "$(PWD)/vos/$(UOUTPUTDIR)/lib/libvos3s5abi.dylib" "$(PWD)/writerfilter/$(UOUTPUTDIR)/lib/libwriterfilter.dylib" "$(PWD)/xmloff/$(UOUTPUTDIR)/LinkTarget/Library/libxo.dylib" "program"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/extensions/$(UOUTPUTDIR)/lib/libupdchk.dylib" "$(PWD)/extensions/$(UOUTPUTDIR)/lib/updchk.uno.dylib" "$(PWD)/jvmfwk/$(UOUTPUTDIR)/lib/sunjavaplugin.dylib" "$(PWD)/pyuno/$(UOUTPUTDIR)/lib/libpyuno.dylib" "program"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/hsqldb/$(UOUTPUTDIR)/misc/build/hsqldb/lib/hsqldb.jar" "program/classes"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice3" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/extensions/$(UOUTPUTDIR)/bin/updchkruninstallers" "MacOS/updchkruninstallers.bin" ; chmod a+x "MacOS/updchkruninstallers.bin"
else ifdef PRODUCT_BUILD2
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice2" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
else
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/vcl/$(UOUTPUTDIR)/LinkTarget/Executable/checknativefont" "program/checknativefont" ; chmod a+x "program/checknativefont"
# Mac App Store will reject apps with shell scripts
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/soffice.bin" ; ln -sf "../MacOS/soffice.bin" "program/soffice.bin"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in sbase scalc sdraw simpress smath soffice swriter unopkg unopkg.bin ; do rm -f "program/$$i" ; ln -sf "soffice.bin" "MacOS/$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in regcomp uno ; do rm -f "program/$$i" ; ln -sf "$$i.bin" "program/$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/resource/dba"*.res ; cp -f "$(PWD)/dbaccess/$(UOUTPUTDIR)/bin/dba"*.res "program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/resource/deploymentgui"*.res ; cp -f "$(PWD)/desktop/$(UOUTPUTDIR)/bin/deploymentgui"*.res "program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/resource/sfx"*.res ; cp -f "$(PWD)/sfx2/$(UOUTPUTDIR)/ResTarget/sfx"*.res "program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/resource/sw"*.res ; cp -f "$(PWD)/sw/$(UOUTPUTDIR)/ResTarget/sw"*.res "program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/resource/vcl"*.res ; cp -f "$(PWD)/vcl/$(UOUTPUTDIR)/ResTarget/vcl"*.res "program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in editpic.pl poll.pl savepic.pl show.pl ; do rm -f "share/config/webcast/$$i" ; cp -f "$(PWD)/sd/res/webview/$$i" "share/config/webcast/$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "share/config/webcast/webcast.pl" ; cp -f "$(PWD)/sd/res/webview/webview.pl" "share/config/webcast/webcast.pl"
ifdef PRODUCT_BUILD2
# Set build version by appending zero instead of date so that it will always be
# less than the Mac App Store version's build version
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/package/Info.plist" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME)#$(PRODUCT_TRADEMARKED_NAME)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(PRODUCT_FILETYPE)#$(PRODUCT_FILETYPE)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(PRODUCT_BUILD_VERSION)#$(PRODUCT_SHORT_VERSION).'"0"'#g' > "Info.plist"
else
# Set build version using date and hour. Mac App Store limits to 10 digits.
# Use base version for build version as Mac App Store requires consistently
# increasing versions.
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/package/Info.plist" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME)#$(PRODUCT_TRADEMARKED_NAME)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(PRODUCT_FILETYPE)#$(PRODUCT_FILETYPE)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(PRODUCT_BUILD_VERSION)#$(PRODUCT_VERSION_BASE).'"`date -u '+%Y%m%d%H'`"'#g' > "Info.plist"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; printf '%s' 'APPL$(PRODUCT_FILETYPE)' > "PkgInfo"
	rm -Rf "$(INSTALL_HOME)/package/Contents/Resources"
	mkdir -p "$(INSTALL_HOME)/package/Contents/Resources/cursors"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/vcl/java/com/sun/star/vcl/images/"*.gif "Resources/cursors"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -c -e 'for i in "$(PWD)/vcl/aqua/source/res/cursors/darc.png" "$(PWD)/vcl/aqua/source/res/cursors/dbezier.png" "$(PWD)/vcl/aqua/source/res/cursors/dcapt.png" "$(PWD)/vcl/aqua/source/res/cursors/dcirccut.png" "$(PWD)/vcl/aqua/source/res/cursors/dconnect.png" "$(PWD)/vcl/aqua/source/res/cursors/dellipse.png" "$(PWD)/vcl/aqua/source/res/cursors/dfree.png" "$(PWD)/vcl/aqua/source/res/cursors/dline.png" "$(PWD)/vcl/aqua/source/res/cursors/dpie.png" "$(PWD)/vcl/aqua/source/res/cursors/dpolygon.png" "$(PWD)/vcl/aqua/source/res/cursors/drect.png" "$(PWD)/vcl/aqua/source/res/cursors/dtext.png" "$(PWD)/vcl/aqua/source/res/cursors/vtext.png" ; do cp "$${i}" "Resources/cursors" ; done'
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; ( ( cd "$(PWD)/vcl/java/source/res" ; gnutar cvf - --exclude CVS MainMenu.nib ) | gnutar xvf - )
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/neo2toolbarv101.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_classic.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_classic.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; find svtools svx -type f > "$(PWD)/$(INSTALL_HOME)/toolbaricons"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/source/Generic Template.icns" "Resources/generic.icns"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/AkuaIcons.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images"
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; unzip "$(PWD)/$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_classic.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_classic.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/NeoOfficeAquaElements.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/about.bmp" "program/about.png" ; cp "tmp/NeoOffice Aqua Elements 3/Contents/MacOS/about.bmp" "program/about.bmp"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/intro.bmp" "program/intro.png" ; cp "$(PWD)/etc/package/intro_classic.bmp" "program/intro.bmp"
else ifdef PRODUCT_BUILD2
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/intro.bmp" "program/intro.png" ; cp "$(PWD)/etc/package/intro_free.bmp" "program/intro.bmp"
else
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/intro.bmp" "program/intro.png" ; cp "tmp/NeoOffice Aqua Elements 3/Contents/MacOS/intro.bmp" "program/intro.bmp"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/logo.png"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; cp "tmp/NeoOffice Aqua Elements 3/Contents/Resources/"*.icns "Resources"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/package/ship.icns" "Resources"
endif
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_classic.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_classic.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"` svtools svx
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; xmllint --noblanks "$(PWD)/etc/program/services.rdb" > "program/services.rdb"
else
	cd "$(INSTALL_HOME)/package/Contents" ; xmllint --noblanks "$(PWD)/etc/sandbox/program/services.rdb" > "program/services.rdb"
endif
# Add Mac OS X localized resources
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; sh -e -c 'for i in `echo "$(PRODUCT_BUNDLED_LANG_PACKS)" | sed "s#-#_#g"` ; do mkdir -p "$${i}.lproj" ; mkdir -p `echo "$${i}" | sed "s#_.*\\$$##"`".lproj" ; done'
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; ( ( cd "$(PWD)/etc/package/l10n" ; gnutar cvf - --exclude CVS --exclude "*.html" . ) | gnutar xvf - )
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; sh -e -c 'for i in `cd "$(PWD)/etc/package/l10n" ; find . -name "*.html"` ; do sed "s#\$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g" "$(PWD)/etc/package/l10n/$${i}" | sed "s#\$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g" | sed "s#\$$(PRODUCT_DOWNLOAD_URL)#$(PRODUCT_DOWNLOAD_URL)#g" | sed "s#\$$(PRODUCT_MIN_OSVERSION_NAME)#$(PRODUCT_MIN_OSVERSION_NAME)#g" | sed "s#\$$(PRODUCT_MAX_OSVERSION_NAME)#$(PRODUCT_MAX_OSVERSION_NAME)#g" > "$${i}" ; done'
endif
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf README* program/LICENSE* program/open-url program/senddoc program/startup.sh program/unoinfo readmes share/extensions/install share/readme
# Fix bug 3273 by not installing any OOo fonts
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "program/fps_aqua.uno.dylib" "program/libMacOSXSpell.dylib" "program/libavmediaQuickTime.dylib" "program/liboooimprovecore.dylib" "share/fonts/truetype" "share/psprint"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/jvmfwk/$(UOUTPUTDIR)/bin/javavendors.xml" "program/javavendors.xml"
else
# Remove update check files since the Mac App Store has its own update
# check. Do not remove updatefeed.uno.dylib as it is needed by the pdfimport
# extension.
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "program/libupdchk.dylib"
# Remove Java files
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "program/classes" "program/libhsqldb.dylib" "program/libjdbc.dylib" "share/Scripts/beanshell" "share/Scripts/java" "program/JREProperties.class" "program/javaloader.uno.dylib" "program/javavm.uno.dylib" "program/javavendors.xml" "program/libjava_uno.dylib" "program/libjuh.dylib" "program/libjuhx.dylib" "program/sunjavaplugin.dylib"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . ! -type d -name "*.jnilib"` ; do rm -f $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents/help" ; sh -e -c 'for i in `find . -type d -name "*.idxl"` ; do rm -Rf $${i} ; done'
# Remove Python files
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "program/libpyuno.dylib" "program/mailmerge.py" "program/officehelper.py" "program/pythonloader.py" "program/pythonscript.py" "program/pythonloader.uno.dylib" "program/pythonloader.unorc" "program/pythonscript.py" "program/pyuno.so" "program/uno.py" "program/unohelper.py" "share/Scripts/python" "share/registry/pyuno.xcd"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share -type f -not -path "*/CVS/*" | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share -type f -name "*.xcd" -not -path "*/CVS/*"` ; do xmllint --noblanks "$(PWD)/etc/$${i}" > "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share -type f -not -name "*.xcd" -not -path "*/CVS/*"` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
ifndef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc/sandbox" ; find share -type f -not -path "*/CVS/*" | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc/sandbox" ; find share -type f -name "*.xcd" -not -path "*/CVS/*"` ; do xmllint --noblanks "$(PWD)/etc/sandbox/$${i}" > "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc/sandbox" ; find share -type f -not -name "*.xcd" -not -path "*/CVS/*"` ; do cp "$(PWD)/etc/sandbox/$${i}" "$${i}" ; done'
# Remove report toolbar from Base since reports require Java support
	rm "$(INSTALL_HOME)/package/Contents/share/config/soffice.cfg/modules/dbapp/toolbar/reportobjectbar.xml"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' | sed 's#BuildMachine=.*$$#BuildMachine=$(BUILD_MACHINE)#g' > "../../out" ; mv -f "../../out" "program/bootstraprc"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(TARGET_MACHINE)#$(TARGET_MACHINE)#g' "$(PWD)/etc/program/fundamentalrc" > "program/fundamentalrc"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/program/sofficerc" "program/sofficerc"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/program/versionrc" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_UPDATE_CHECK_URL)#$(PRODUCT_UPDATE_CHECK_URL)#g' | sed 's# #%20#g' | sed 's#^buildid=.*$$#buildid=$(PRODUCT_PATCH_VERSION)#' > "program/versionrc"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "share/registry/main.xcd" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "../../out" ; mv -f "../../out" "share/registry/main.xcd"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(OO_PRODUCT_NAME)#$(OO_PRODUCT_NAME)#g' "$(PWD)/etc/help/main_transform.xsl" | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL)#$(PRODUCT_DOCUMENTATION_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL_TEXT)#$(PRODUCT_DOCUMENTATION_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#g' > "help/main_transform.xsl"
else
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(OO_PRODUCT_NAME)#$(OO_PRODUCT_NAME)#g' "$(PWD)/etc/sandbox/help/main_transform.xsl" | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL)#$(PRODUCT_DOCUMENTATION_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL_TEXT)#$(PRODUCT_DOCUMENTATION_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#g' > "help/main_transform.xsl"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.bin"` ; do strip -S -x "$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*"` ; do strip -S -x "$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.so"` ; do strip -S -x "$$i" ; done'
# Shared libraries in extensions may link to OpenOffice shared libraries using
# @executable_path so create softlinks in the MacOS directory for each shared
# library in the program directory
	cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -e -c 'for i in `find "../program" -type f -name "*.dylib*"` ; do ln -sf "$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -e -c 'for i in `find "../program" -type f -name "*.so"` ; do ln -sf "$$i" ; done'
ifdef PRODUCT_BUILD3
# Install OOo .oxt files
	cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -c -e 'JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1 ; export JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY ; unset CLASSPATH ; unset DYLD_LIBRARY_PATH ; for i in `find "$(PWD)/$(OO_BUILD_HOME)/../ext_sources" -type f -name "*.oxt"` "$(PWD)/sdext/$(UOUTPUTDIR)/bin/pdfimport.oxt" ; do rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp" ; echo "yes" | ./unopkg.bin add --shared --verbose "$$i" -env:UserInstallation=file://"$(PWD)/$(INSTALL_HOME)/tmp" ; done ; rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp"'
# Install report-builder.oxt
	cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -c -e 'JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1 ; export JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY ; unset CLASSPATH ; unset DYLD_LIBRARY_PATH ; for i in `find "$(PWD)/$(OO_BUILD_HOME)/solver/412/$(UOUTPUTDIR)/bin" -type f -name "report-builder.oxt"` ; do rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp" ; echo "yes" | ./unopkg.bin add --shared --verbose "$$i" -env:UserInstallation=file://"$(PWD)/$(INSTALL_HOME)/tmp" ; done ; rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp"'
else
# Install OOo .oxt files. Exclude lo-oo-ressources-linguistiques-fr-v5.3.oxt
# and lightproof-hu_HU-1.3.oxt since they require Python.
	cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -c -e 'JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1 ; export JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY ; unset CLASSPATH ; unset DYLD_LIBRARY_PATH ; for i in `find "$(PWD)/$(OO_BUILD_HOME)/../ext_sources" -type f -name "*.oxt" | grep -v lo-oo-ressources-linguistiques-fr-v5.3.oxt | grep -v lightproof-hu_HU-1.3.oxt` "$(PWD)/sdext/$(UOUTPUTDIR)/bin/pdfimport.oxt" ; do rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp" ; echo "yes" | ./unopkg.bin add --shared --verbose "$$i" -env:UserInstallation=file://"$(PWD)/$(INSTALL_HOME)/tmp" ; done ; rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp"'
endif
# Install shared .oxt files
	cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -c -e 'JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1 ; export JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY ; unset CLASSPATH ; unset DYLD_LIBRARY_PATH ; for i in `echo "$(PRODUCT_COMPONENT_MODULES)"` ; do if [ -f "$(PWD)/$$i/$(UOUTPUTDIR)/bin/$$i.oxt" ] ; then rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp" ; ./unopkg.bin add --shared --verbose "$(PWD)/$$i/$(UOUTPUTDIR)/bin/$$i.oxt" -env:UserInstallation=file://"$(PWD)/$(INSTALL_HOME)/tmp" ; fi ; done ; rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp"'
	mkdir -p "$(INSTALL_HOME)/package/Contents/Library/Spotlight"
	cd "$(INSTALL_HOME)/package/Contents/Library/Spotlight" ; tar zxvf "$(PWD)/$(NEOOFFICE_PATCHES_HOME)/neolight.mdimporter.tgz"
# Make Spotlight plugin ID unique for each build. Fix bug 2711 by updating
# plugin bundle IDs.
	cd "$(INSTALL_HOME)/package/Contents/Library/SpotLight" ; sed 's#$(NEOLIGHT_MDIMPORTER_ID)#$(NEOLIGHT_MDIMPORTER_ID).$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).'"`date '+%Y%m%d%H%M%S'`"'#g' "neolight.mdimporter/Contents/Info.plist" > "../../out" ; mv -f "../../out" "neolight.mdimporter/Contents/Info.plist"
	mkdir -p "$(INSTALL_HOME)/package/Contents/Library/QuickLook"
	cd "$(INSTALL_HOME)/package/Contents/Library/QuickLook" ; tar zxvf "$(PWD)/$(NEOOFFICE_PATCHES_HOME)/neopeek.qlgenerator.tgz"
# Make QL plugin ID unique for each build. Fix bug 2711 by updating plugin
# bundle IDs.
	cd "$(INSTALL_HOME)/package/Contents/Library/QuickLook" ; sed 's#$(NEOPEEK_QLPLUGIN_ID)#$(NEOPEEK_QLPLUGIN_ID).$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).'"`date '+%Y%m%d%H%M%S'`"'#g' "neopeek.qlgenerator/Contents/Info.plist" > "../../out" ; mv -f "../../out" "neopeek.qlgenerator/Contents/Info.plist"
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find "." -name ".DS_Store"` ; do rm "$${i}" ; done'
	xattr -rcs "$(INSTALL_HOME)/package"
# Sign all binaries
	chmod -Rf u+rw "$(INSTALL_HOME)/package"
# Mac App Store will reject apps with shell scripts
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'if find . ! -type d -exec file {} \; | grep "script text executable" ; then exit 1 ; fi'
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type d -name "*.mdimporter"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type d -name "*.qlgenerator"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
# Sign "A" version of each framework
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find "Contents/Frameworks" -type d -name "A"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type f -name "*.dylib*"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type f -name "*.so"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" "Contents/NOTICE"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" "Contents/MacOS/updchkruninstallers.bin"
	cd "$(INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" .
else
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type f -name "*.bin" \! -name "soffice.bin"` "Contents/program/checknativefont" "Contents/program/pagein" "Contents/program/regmerge" "Contents/program/regview" "Contents/program/uri-encode" `find "Contents/share/uno_packages/cache/uno_packages" -type f -name "xpdfimport"` ; do codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/etc/package/Entitlements_inherit_only.plist" "$$i" ; done'
	cat "etc/package/Entitlements.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(INSTALL_HOME)/Entitlements.plist"
	cd "$(INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/$(INSTALL_HOME)/Entitlements.plist" .
endif
# Test that all libraries will load. Exclude all of the CoinMP libraries as
# they will fail to load so we only care if the OpenOffice libraries that link
# to them load.
	cd "$(INSTALL_HOME)/package" ; $(CC) -arch "$(TARGET_MACHINE)" -o "Contents/MacOS/loaddyliblist" "$(PWD)/etc/package/loaddyliblist.c" ; sh -e -c 'find . -type f -name "*.dylib*" -not -path "*/libCoinMP[.0-9]*" -not -path "*/libCoinUtils[.0-9]*" -not -path "*/libClp[.0-9]*" -not -path "*/libCbc[.0-9]*" -not -path "*/libOsi[.0-9]*" -not -path "*/libOsiCbc[.0-9]*" -not -path "*/libOsiClp[.0-9]*" -not -path "*/libCgl[.0-9]*" -not -path "*/libCbcSolver[.0-9]*" | "Contents/MacOS/loaddyliblist" ; rm -f "Contents/MacOS/loaddyliblist"'
	mkdir -p "$(INSTALL_HOME)/tmp"
	mkdir "$(INSTALL_HOME)/package/$(PRODUCT_INSTALL_DIR_NAME).app"
	mv -f "$(INSTALL_HOME)/package/Contents" "$(INSTALL_HOME)/package/$(PRODUCT_INSTALL_DIR_NAME).app"
# Mac App Store requires files to be writable by root
	chmod -Rf u+w,og-w,a+r "$(INSTALL_HOME)/package"
# Mark certain directories writable for group
	chmod -f 775 "$(INSTALL_HOME)/package/$(PRODUCT_INSTALL_DIR_NAME).app"
	chmod -f 775 "$(INSTALL_HOME)/package/$(PRODUCT_INSTALL_DIR_NAME).app/Contents/Resources"
	echo "Running sudo to chown installation files..."
	sudo chown -Rf root:admin "$(INSTALL_HOME)/package"
	mkdir -p "$(INSTALL_HOME)/package.pkg/Resources"
	mkdir -p "$(INSTALL_HOME)/package.pkg/contents.pkg"
	cd "$(INSTALL_HOME)/package.pkg/Resources" ; sh -e -c 'for i in `cd "/System/Library/PrivateFrameworks/Install.framework/Resources" ; find . -type d -name "*.lproj" -maxdepth 1` ; do mkdir -p "$${i}" ; done'
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/PackageInfo" | sed 's#$$(PRODUCT_INSTALL_DIR_NAME)#$(PRODUCT_INSTALL_DIR_NAME)#g' | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Distribution" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(TARGET_MACHINE)#$(TARGET_MACHINE)#g' > "$(INSTALL_HOME)/package.pkg/Distribution"
ifdef PRODUCT_BUILD3
	echo '<domains enable_anywhere="true"/>' >> "$(INSTALL_HOME)/package.pkg/Distribution"
else ifdef PRODUCT_BUILD2
	echo '<domains enable_localSystem="true"/>' >> "$(INSTALL_HOME)/package.pkg/Distribution"
else
	echo '<domains enable_localSystem="true"/>' >> "$(INSTALL_HOME)/package.pkg/Distribution"
endif
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cp "etc/package/ship.tiff" "$(INSTALL_HOME)/package.pkg/Resources/background.tiff"
	echo '<background file="background.tiff" alignment="bottomleft" scaling="proportional"/>' >> "$(INSTALL_HOME)/package.pkg/Distribution"
endif
	mkbom "$(INSTALL_HOME)/package" "$(INSTALL_HOME)/package.pkg/contents.pkg/Bom" >& /dev/null
	( cd "$(INSTALL_HOME)/package" ; pax -w -z -x cpio . ) > "$(INSTALL_HOME)/package.pkg/contents.pkg/Payload"
ifdef PRODUCT_BUILD3
	echo '<strict-identifier/>' >> "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	echo '<scripts><postinstall file="./postflight"/></scripts>' >> "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
else ifdef PRODUCT_BUILD2
	echo '<strict-identifier><bundle id="$(PRODUCT_DOMAIN).$(PRODUCT_DIR_NAME)"/></strict-identifier>' >> "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	echo '<scripts><postinstall file="./postflight"/></scripts>' >> "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
else
	echo '<strict-identifier><bundle id="$(PRODUCT_DOMAIN).$(PRODUCT_DIR_NAME)"/></strict-identifier>' >> "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
endif
	echo '<payload installKBytes="'`du -sk "$(INSTALL_HOME)/package" | awk '{ print $$1 }'`'" numberOfFiles="'`lsbom "$(INSTALL_HOME)/package.pkg/contents.pkg/Bom" | wc -l`'"/>' >> "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	echo '</pkg-info>' >> "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	cd "bin" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do mkdir -p "$${i}" ; cat "$(PWD)/bin/$${i}/InstallationCheck.strings" | sed "s#\$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g" | sed "s#\$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g" | sed "s#\$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(PREFLIGHT_REQUIRED_COMMANDS)#g" > "$(PWD)/$(INSTALL_HOME)/package.pkg/Resources/$${i}/Localizable.strings" ; done'
	cd "$(INSTALL_HOME)/package.pkg/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do if [ ! -e "$${i}/Localizable.strings" ] ; then cp -f "English.lproj/Localizable.strings" "$${i}/Localizable.strings" ; fi ; done'
ifdef PRODUCT_BUILD3
	mkdir -p "$(INSTALL_HOME)/package.pkg/contents.pkg/Scripts"
	cat "bin/postflight" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' > "$(INSTALL_HOME)/package.pkg/contents.pkg/Scripts/postflight" ; chmod a+x "$(INSTALL_HOME)/package.pkg/contents.pkg/Scripts/postflight"
else ifdef PRODUCT_BUILD2
	mkdir -p "$(INSTALL_HOME)/package.pkg/contents.pkg/Scripts"
	cat "bin/postflight" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' > "$(INSTALL_HOME)/package.pkg/contents.pkg/Scripts/postflight" ; chmod a+x "$(INSTALL_HOME)/package.pkg/contents.pkg/Scripts/postflight"
endif
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
	cat "etc/ReadMe.rtf" | sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME_RTF)#'"$(PRODUCT_TRADEMARKED_NAME_RTF)"'#g' | sed 's#$$(PRODUCT_BASE_URL)#'"$(PRODUCT_BASE_URL)"'#g' | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)/ReadMe.rtf"
	echo '</installer-gui-script>' >> "$(INSTALL_HOME)/package.pkg/Distribution"
	pkgutil --flatten "$(INSTALL_HOME)/package.pkg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION).pkg"
# Sign package
	mv -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION).pkg" "$(INSTALL_HOME)/unsigned.pkg"
	productsign --sign "$(CERTPKGIDENTITY)" "$(INSTALL_HOME)/unsigned.pkg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION).pkg"
	rm -f "$(INSTALL_HOME)/unsigned.pkg"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	rm -Rf "$(INSTALL_HOME)/tmp"
	mkdir -p "$(INSTALL_HOME)/tmp"
	cd "$(INSTALL_HOME)/tmp" ; unzip "$(PWD)/etc/package/SetFileIcon.zip" ; $(CC) -o "SetFileIcon/SetFileIcon" -framework AppKit "SetFileIcon/SetFileIcon.m" ; "SetFileIcon/SetFileIcon" -image "$(PWD)/etc/package/ship.icns" -file "$(PWD)/$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION).pkg"
	rm -Rf "$(INSTALL_HOME)/tmp"
endif
	chmod -Rf a-w,a+r "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
	chmod -f u+w "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
# Use YourSway fancy .dmg tool to create .dmg file
	rm -Rf "$(INSTALL_HOME)/tmp"
	mkdir -p "$(INSTALL_HOME)/tmp"
	cd "$(INSTALL_HOME)/tmp" ; unzip "$(PWD)/etc/package/$(YOURSWAYCREATEDMG_SOURCE_FILENAME)"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	sync ; "$(INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" --volname "Install $(PRODUCT_NAME) $(PRODUCT_VERSION)" --volicon "etc/package/ship.icns" --icon-size 128 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION).pkg" 150 100 --icon "ReadMe.rtf" 350 100 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION).pkg" 150 100 --icon "ReadMe.rtf" 350 100 --window-pos 400 300 --window-size 500 250 "$(INSTALL_HOME)/$(PRODUCT_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).dmg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
else
	sync ; "$(INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).dmg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
endif
	rm -Rf "$(INSTALL_HOME)/tmp"

build.patch_package: build.package
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" -o "$$PRODUCT_DOMAIN" != "$(PRODUCT_DOMAIN)" -o "$$PRODUCT_DIR_NAME" != "$(PRODUCT_DIR_NAME)" -o "$$PRODUCT_MAC_APP_STORE_URL" != "$(PRODUCT_MAC_APP_STORE_URL)" -o "$$PRODUCT_JAVA_DOWNLOAD_URL" != "$(PRODUCT_JAVA_DOWNLOAD_URL)" -o "$$PRODUCT_MIN_OSVERSION" != "$(PRODUCT_MIN_OSVERSION)" -o "$$PRODUCT_MAX_OSVERSION" != "$(PRODUCT_MAX_OSVERSION)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	"$(MAKE)" $(MFLAGS) "CERTAPPIDENTITY=$(PATCHCERTAPPIDENTITY)" "CERTPKGIDENTITY=$(PATCHCERTPKGIDENTITY)" "build.patch_package_shared"
	touch "$@"

build.patch_package2: build.package2
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" -o "$$PRODUCT_DOMAIN" != "$(PRODUCT_DOMAIN)" -o "$$PRODUCT_DIR_NAME2" != "$(PRODUCT_DIR_NAME2)" -o "$$PRODUCT_MAC_APP_STORE_URL" != "$(PRODUCT_MAC_APP_STORE_URL)" -o "$$PRODUCT_JAVA_DOWNLOAD_URL" != "$(PRODUCT_JAVA_DOWNLOAD_URL)" -o "$$PRODUCT_MIN_OSVERSION" != "$(PRODUCT_MIN_OSVERSION)" -o "$$PRODUCT_MAX_OSVERSION" != "$(PRODUCT_MAX_OSVERSION)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
ifndef PRODUCT_BUILD2
	"$(MAKE)" $(MFLAGS) "PRODUCT_BUILD2=TRUE" "PRODUCT_VERSION=$(PRODUCT_VERSION2)" "PRODUCT_VERSION_EXT=$(PRODUCT_VERSION_EXT2)" "PRODUCT_DIR_NAME=$(PRODUCT_DIR_NAME2)" "CERTAPPIDENTITY=$(PATCHCERTAPPIDENTITY2)" "CERTPKGIDENTITY=$(PATCHCERTPKGIDENTITY2)" "PRODUCT_BUNDLED_LANG_PACKS=$(PRODUCT_BUNDLED_LANG_PACKS2)" "build.patch_package_shared"
	touch "$@"
endif

build.patch_package3: build.package3
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" -o "$$PRODUCT_DOMAIN" != "$(PRODUCT_DOMAIN)" -o "$$PRODUCT_DIR_NAME3" != "$(PRODUCT_DIR_NAME3)" -o "$$PRODUCT_MAC_APP_STORE_URL" != "$(PRODUCT_MAC_APP_STORE_URL)" -o "$$PRODUCT_JAVA_DOWNLOAD_URL" != "$(PRODUCT_JAVA_DOWNLOAD_URL)" -o "$$PRODUCT_MIN_OSVERSION" != "$(PRODUCT_MIN_OSVERSION)" -o "$$PRODUCT_MAX_OSVERSION" != "$(PRODUCT_MAX_OSVERSION)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
ifndef PRODUCT_BUILD3
	"$(MAKE)" $(MFLAGS) "PRODUCT_BUILD3=TRUE" "PRODUCT_VERSION=$(PRODUCT_VERSION3)" "PRODUCT_VERSION_EXT=$(PRODUCT_VERSION_EXT3)" "PRODUCT_DIR_NAME=$(PRODUCT_DIR_NAME3)" "CERTAPPIDENTITY=$(PATCHCERTAPPIDENTITY3)" "CERTPKGIDENTITY=$(PATCHCERTPKGIDENTITY3)" "PRODUCT_BUNDLED_LANG_PACKS=$(PRODUCT_BUNDLED_LANG_PACKS3)" "build.patch_package_shared"
	touch "$@"
endif

build.patch_package_shared:
# Check that codesign and productsign executables exist before proceeding
	@sh -e -c 'for i in codesign productsign ; do if [ -z "`which $$i`" ] ; then echo "$$i command not found" ; exit 1 ; fi ; done'
	sh -e -c 'if [ -d "$(PATCH_INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(PATCH_INSTALL_HOME)" ; fi'
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/MacOS"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/program"
# Copy all resource files in the main installer and overwrite newer resources
# so that the codesigning will not remove resource files marked as signed in an
# existing installation
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/Resources"
	cd "$(PATCH_INSTALL_HOME)/package/Contents/Resources" ; ( ( cd "$(PWD)/$(INSTALL_HOME)/package/$(PRODUCT_INSTALL_DIR_NAME).app/Contents/Resources" ; find . \! -type d -print0 | xargs -0 gnutar cvf - ) | gnutar xvf - );
ifdef PRODUCT_BUILD3
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice3" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
else ifdef PRODUCT_BUILD2
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice2" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
else
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
endif
	chmod -Rf u+w,a+r "$(PATCH_INSTALL_HOME)/package"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/vcl/$(UOUTPUTDIR)/LinkTarget/Library/libvcl.dylib" "program"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/package/Info.plist" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME)#$(PRODUCT_TRADEMARKED_NAME)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(PRODUCT_FILETYPE)#$(PRODUCT_FILETYPE)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(PRODUCT_BUILD_VERSION)#$(PRODUCT_VERSION_BASE)#g' > "Info.plist"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' | sed 's#BuildMachine=.*$$#BuildMachine=$(BUILD_MACHINE)#g' > "../../out" ; mv -f "../../out" "program/bootstraprc"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/program/versionrc" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_UPDATE_CHECK_URL)#$(PRODUCT_UPDATE_CHECK_URL)#g' | sed 's# #%20#g' | sed 's#^buildid=.*$$#buildid=$(PRODUCT_PATCH_VERSION)#' > "program/versionrc"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.bin"` ; do strip -S -x "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*"` ; do strip -S -x "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.so"` ; do strip -S -x "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find "." -name ".DS_Store"` ; do rm "$${i}" ; done'
	xattr -rcs "$(PATCH_INSTALL_HOME)/package"
# Sign all binaries and use code resources file from main installer so that an
# updated code resources is created without reshipping all referenced files
	chmod -Rf u+rw "$(PATCH_INSTALL_HOME)/package"
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type d -name "*.mdimporter"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type d -name "*.qlgenerator"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
# Sign "A" version of each framework
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find "Contents/Frameworks" -type d -name "A"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type f -name "*.dylib*"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type f -name "*.so"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
ifdef PRODUCT_BUILD3
	cd "$(PATCH_INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" .
else
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type f -name "*.bin" \! -name "soffice.bin"` ; do codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/etc/package/Entitlements_inherit_only.plist" "$$i" ; done'
	cat "etc/package/Entitlements.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(PATCH_INSTALL_HOME)/Entitlements.plist"
	cd "$(PATCH_INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/$(PATCH_INSTALL_HOME)/Entitlements.plist" .
endif
# Mac App Store requires files to be writable by root
	chmod -Rf u+w,og-w,a+r "$(PATCH_INSTALL_HOME)/package"
# Mark certain directories writable for group
	chmod -f 775 "$(PATCH_INSTALL_HOME)/package"
	chmod -f 775 "$(PATCH_INSTALL_HOME)/package/Contents/Resources"
	echo "Running sudo to chown installation files..."
	sudo chown -Rf root:admin "$(PATCH_INSTALL_HOME)/package"
	mkdir -p "$(PATCH_INSTALL_HOME)/package.pkg/Resources"
	mkdir -p "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Scripts"
	cd "$(PATCH_INSTALL_HOME)/package.pkg/Resources" ; sh -e -c 'for i in `cd "/System/Library/PrivateFrameworks/Install.framework/Resources" ; find . -type d -name "*.lproj" -maxdepth 1` ; do mkdir -p "$${i}" ; done'
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/PackageInfo.patch" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Distribution.patch" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(TARGET_MACHINE)#$(TARGET_MACHINE)#g' > "$(PATCH_INSTALL_HOME)/package.pkg/Distribution"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cp "etc/package/ship.tiff" "$(PATCH_INSTALL_HOME)/package.pkg/Resources/background.tiff"
	echo '<background file="background.tiff" alignment="bottomleft" scaling="proportional"/>' >> "$(PATCH_INSTALL_HOME)/package.pkg/Distribution"
endif
# Copy shared .oxt files
# Sign all binaries in .oxt files
	rm -Rf "$(PATCH_INSTALL_HOME)/tmp"
	mkdir -p "$(PATCH_INSTALL_HOME)/tmp"
	cd "$(PATCH_INSTALL_HOME)/tmp" ; sh -e -c 'for i in `echo "$(PRODUCT_COMPONENT_PATCH_MODULES)"` ; do if [ -f "$(PWD)/$$i/$(UOUTPUTDIR)/bin/$$i.oxt" ] ; then mkdir "$$i" ; ( cd "$$i" ; unzip "$(PWD)/$$i/$(UOUTPUTDIR)/bin/$$i.oxt" ; for j in `find . -type f -name "*.dylib*"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$j" ; done ; zip -r "$(PWD)/$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Scripts/$$i.oxt" . ) ; rm -Rf "$$i" ; fi ; done'
	rm -Rf "$(PATCH_INSTALL_HOME)/tmp"
# Make empty BOM so that nothing gets extracted in the temporary installation
	mkdir "$(PATCH_INSTALL_HOME)/emptydir"
	mkbom "$(PATCH_INSTALL_HOME)/emptydir" "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Bom" >& /dev/null
	( cd "$(PATCH_INSTALL_HOME)/emptydir" ; pax -w -z -x cpio . ) > "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Payload"
	( cd "$(PATCH_INSTALL_HOME)/package" ; pax -w -z -x cpio . ) > "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Scripts/Archive.pax.gz"
	rm -Rf "$(PATCH_INSTALL_HOME)/emptydir"
	echo '<payload installKBytes="'`du -sk "$(PATCH_INSTALL_HOME)/package" | awk '{ print $$1 }'`'" numberOfFiles="'`lsbom "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Bom" | wc -l`'"/>' >> "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	echo '</pkg-info>' >> "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils.patch" | sed 's#$$(PRODUCT_USER_INSTALL_DIR)#$(PRODUCT_USER_INSTALL_DIR)#g' | sed 's#$$(PRODUCT_VERSION_BASE)#$(PRODUCT_VERSION_BASE)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_VERSION_EXT)#$(PRODUCT_VERSION_EXT)#g' | sed 's#$$(PREVIOUS_PRODUCT_VERSION)#$(PREVIOUS_PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(TARGET_FILE_TYPE)#$(TARGET_FILE_TYPE)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(PATCH_INSTALL_HOME)/installutils"
	cat "$(PATCH_INSTALL_HOME)/installutils" "bin/InstallationCheck" | sed 's#$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g' > "$(PATCH_INSTALL_HOME)/InstallationCheck" ; chmod a+x "$(PATCH_INSTALL_HOME)/InstallationCheck"
	cat "$(PATCH_INSTALL_HOME)/installutils" "bin/VolumeCheck.patch" > "$(PATCH_INSTALL_HOME)/VolumeCheck" ; chmod a+x "$(PATCH_INSTALL_HOME)/VolumeCheck"
	cd "bin" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do mkdir -p "$${i}" ; cat "$(PWD)/bin/$${i}/InstallationCheck.strings" "$(PWD)/bin/$${i}/VolumeCheck.strings.patch" | sed "s#\$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g" | sed "s#\$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g" | sed "s#\$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g" > "$(PWD)/$(PATCH_INSTALL_HOME)/package.pkg/Resources/$${i}/Localizable.strings" ; done'
	cd "$(PATCH_INSTALL_HOME)/package.pkg/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do if [ ! -e "$${i}/Localizable.strings" ] ; then cp -f "English.lproj/Localizable.strings" "$${i}/Localizable.strings" ; fi ; done'
	sh -e "etc/convertscripttojsstring.sh" "$(PATCH_INSTALL_HOME)/InstallationCheck" > "$(PATCH_INSTALL_HOME)/InstallationCheck.js"
	sh -e "etc/convertscripttojsstring.sh" "$(PATCH_INSTALL_HOME)/VolumeCheck" > "$(PATCH_INSTALL_HOME)/VolumeCheck.js"
	sed 's#$$(PRODUCT_NAME_AND_VERSION)#$(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION)#g' "etc/Distribution.js" | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g' | sh -e -c 'sed "s#var.*installationCheckBashScript.*=.*;#var installationCheckBashScript = unescape(\""`cat "$(PATCH_INSTALL_HOME)/InstallationCheck.js"`"\");#" | sed "s#var.*volumeCheckBashScript.*=.*;#var volumeCheckBashScript = unescape(\""`cat "$(PATCH_INSTALL_HOME)/VolumeCheck.js"`"\");#"' >> "$(PATCH_INSTALL_HOME)/package.pkg/Distribution"
	cat "$(PATCH_INSTALL_HOME)/installutils" "bin/preflight.patch" > "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Scripts/preflight" ; chmod a+x "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Scripts/preflight"
	cat "$(PATCH_INSTALL_HOME)/installutils" "bin/postflight.patch" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Scripts/postflight" ; chmod a+x "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Scripts/postflight"
	mkdir -p "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
	echo '</installer-gui-script>' >> "$(PATCH_INSTALL_HOME)/package.pkg/Distribution"
	pkgutil --flatten "$(PATCH_INSTALL_HOME)/package.pkg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION).pkg"
# Sign package
	mv -f "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION).pkg" "$(PATCH_INSTALL_HOME)/unsigned.pkg"
	productsign --sign "$(CERTPKGIDENTITY)" "$(PATCH_INSTALL_HOME)/unsigned.pkg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION).pkg"
	rm -f "$(PATCH_INSTALL_HOME)/unsigned.pkg"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	rm -Rf "$(PATCH_INSTALL_HOME)/tmp"
	mkdir -p "$(PATCH_INSTALL_HOME)/tmp"
	cd "$(PATCH_INSTALL_HOME)/tmp" ; unzip "$(PWD)/etc/package/SetFileIcon.zip" ; $(CC) -o "SetFileIcon/SetFileIcon" -framework AppKit "SetFileIcon/SetFileIcon.m" ; "SetFileIcon/SetFileIcon" -image "$(PWD)/etc/package/ship.icns" -file "$(PWD)/$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION).pkg"
	rm -Rf "$(PATCH_INSTALL_HOME)/tmp"
endif
	chmod -Rf a-w,a+r "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
	chmod -f u+w "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
# Use YourSway fancy .dmg tool to create .dmg file
	rm -Rf "$(PATCH_INSTALL_HOME)/tmp"
	mkdir -p "$(PATCH_INSTALL_HOME)/tmp"
	cd "$(PATCH_INSTALL_HOME)/tmp" ; unzip "$(PWD)/etc/package/$(YOURSWAYCREATEDMG_SOURCE_FILENAME)"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	sync ; "$(PATCH_INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" --volname "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION)" --volicon "etc/package/ship.icns" --icon-size 128 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION).pkg" 250 100 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION).pkg" 250 100 --window-pos 400 300 --window-size 500 250 "$(PATCH_INSTALL_HOME)/$(PRODUCT_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).dmg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
else
	sync ; "$(PATCH_INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).dmg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
endif
	rm -Rf "$(PATCH_INSTALL_HOME)/tmp"

build.package_%: $(INSTALL_HOME)/package_%
	chmod -Rf u+w,a+r "$<"
	cd "$</Contents" ; rm -Rf readmes share/readme
	cd "$</Contents" ; rm -f "program/resource/dba"*.res
	cd "$</Contents" ; rm -f "program/resource/deploymentgui"*.res
	cd "$</Contents" ; rm -f "program/resource/sfx"*.res
	cd "$</Contents" ; rm -f "program/resource/sw"*.res
	cd "$</Contents" ; rm -f "program/resource/vcl"*.res
	cd "$</Contents" ; xmllint --noblanks "$(PWD)/etc/share/registry/res/fcfg_langpack_$*" > "share/registry/res/fcfg_langpack_$*"
ifndef PRODUCT_BUILD3
	cd "$</Contents" ; xmllint --noblanks "$(PWD)/etc/sandbox/share/registry/res/fcfg_langpack_$*" > "share/registry/res/fcfg_langpack_$*"
	cd "$</Contents" ; xmllint --noblanks "$(PWD)/etc/sandbox/share/registry/res/registry_$*" > "share/registry/res/registry_$*"
endif
	rm -Rf "$</Contents/Resources"
	mkdir -p "$</Contents/Resources"
# Add Mac OS X localized resources
	cd "$</Contents/Resources" ; sh -e -c 'for i in `echo "$(PRODUCT_LANG_PACK_LOCALE)" | sed "s#-#_#g"` ; do mkdir -p "$${i}.lproj" ; mkdir -p `echo "$${i}" | sed "s#_.*\\$$##"`".lproj" ; done'
	cd "$<" ; sh -e -c 'for i in `find "." -name ".DS_Store"` ; do rm "$${i}" ; done'
	xattr -rcs "$<"
# Mac App Store requires files to be writable by root
	chmod -Rf u+w,og-w,a+r "$<"
# Mark certain directories writable for group
	chmod -f 775 "$<"
	chmod -f 775 "$</Contents/Resources"
	echo "Running sudo to chown $(@:build.package_%=%) installation files..."
	sudo chown -Rf root:admin "$<"
	mkdir -p "$<.pkg/Resources"
	mkdir -p "$<.pkg/contents.pkg/Scripts"
	cd "$<.pkg/Resources" ; sh -e -c 'for i in `cd "/System/Library/PrivateFrameworks/Install.framework/Resources" ; find . -type d -name "*.lproj" -maxdepth 1` ; do mkdir -p "$${i}" ; done'
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/PackageInfo.langpack" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_LANG_PACK_VERSION)#$(PRODUCT_LANG_PACK_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$<.pkg/contents.pkg/PackageInfo"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Distribution.langpack" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_LANG_PACK_VERSION)#$(PRODUCT_LANG_PACK_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(TARGET_MACHINE)#$(TARGET_MACHINE)#g' > "$<.pkg/Distribution"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cp "etc/package/ship.tiff" "$<.pkg/Resources/background.tiff"
	echo '<background file="background.tiff" alignment="bottomleft" scaling="proportional"/>' >> "$<.pkg/Distribution"
endif
# Make empty BOM so that nothing gets extracted in the temporary installation
	mkdir "$(INSTALL_HOME)/emptydir"
	mkbom "$(INSTALL_HOME)/emptydir" "$<.pkg/contents.pkg/Bom" >& /dev/null
	( cd "$(INSTALL_HOME)/emptydir" ; pax -w -z -x cpio . ) > "$<.pkg/contents.pkg/Payload"
	( cd "$<" ; pax -w -z -x cpio . ) > "$<.pkg/contents.pkg/Scripts/Archive.pax.gz"
	rm -Rf "$(INSTALL_HOME)/emptydir"
	echo '<payload installKBytes="'`du -sk "$<" | awk '{ print $$1 }'`'" numberOfFiles="'`lsbom "$<.pkg/contents.pkg/Bom" | wc -l`'"/>' >> "$<.pkg/contents.pkg/PackageInfo"
	echo '</pkg-info>' >> "$<.pkg/contents.pkg/PackageInfo"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils.langpack" | sed 's#$$(PRODUCT_USER_INSTALL_DIR)#$(PRODUCT_USER_INSTALL_DIR)#g' | sed 's#$$(PRODUCT_VERSION_BASE)#$(PRODUCT_VERSION_BASE)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_VERSION_EXT)#$(PRODUCT_VERSION_EXT)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PREVIOUS_PRODUCT_VERSION_BASE)#$(PREVIOUS_PRODUCT_VERSION_BASE)#g' | sed 's#$$(TARGET_FILE_TYPE)#$(TARGET_FILE_TYPE)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(INSTALL_HOME)/installutils.langpack"
	cat "$(INSTALL_HOME)/installutils.langpack" "bin/InstallationCheck" | sed 's#$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g' > "$(INSTALL_HOME)/InstallationCheck.langpack" ; chmod a+x "$(INSTALL_HOME)/InstallationCheck.langpack"
	cat "$(INSTALL_HOME)/installutils.langpack" "bin/VolumeCheck.langpack" > "$(INSTALL_HOME)/VolumeCheck.langpack" ; chmod a+x "$(INSTALL_HOME)/VolumeCheck.langpack"
	cd "bin" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do mkdir -p "$${i}" ; cat "$(PWD)/bin/$${i}/InstallationCheck.strings" "$(PWD)/bin/$${i}/VolumeCheck.strings.langpack" | sed "s#\$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g" | sed "s#\$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g" | sed "s#\$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g" > "$(PWD)/$<.pkg/Resources/$${i}/Localizable.strings" ; done'
	cd "$<.pkg/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do if [ ! -e "$${i}/Localizable.strings" ] ; then cp -f "English.lproj/Localizable.strings" "$${i}/Localizable.strings" ; fi ; done'
	sh -e "etc/convertscripttojsstring.sh" "$(INSTALL_HOME)/InstallationCheck.langpack" > "$(INSTALL_HOME)/InstallationCheck.js.langpack"
	sh -e "etc/convertscripttojsstring.sh" "$(INSTALL_HOME)/VolumeCheck.langpack" > "$(INSTALL_HOME)/VolumeCheck.js.langpack"
	sed 's#$$(PRODUCT_NAME_AND_VERSION)#$(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_LANG_PACK_VERSION)#g' "etc/Distribution.js" | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g' | sh -e -c 'sed "s#var.*installationCheckBashScript.*=.*;#var installationCheckBashScript = unescape(\""`cat "$(INSTALL_HOME)/InstallationCheck.js.langpack"`"\");#" | sed "s#var.*volumeCheckBashScript.*=.*;#var volumeCheckBashScript = unescape(\""`cat "$(INSTALL_HOME)/VolumeCheck.js.langpack"`"\");#"' >> "$<.pkg/Distribution"
	cat "$(INSTALL_HOME)/installutils.langpack" "bin/preflight.langpack" > "$<.pkg/contents.pkg/Scripts/preflight" ; chmod a+x "$<.pkg/contents.pkg/Scripts/preflight"
	cat "$(INSTALL_HOME)/installutils.langpack" "bin/postflight.langpack" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION_EXT)#$(PRODUCT_VERSION_EXT)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$<.pkg/contents.pkg/Scripts/postflight" ; chmod a+x "$<.pkg/contents.pkg/Scripts/postflight"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)"
	cat "etc/ReadMe.rtf" | sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME_RTF)#'"$(PRODUCT_TRADEMARKED_NAME_RTF)"'#g' | sed 's#$$(PRODUCT_BASE_URL)#'"$(PRODUCT_BASE_URL)"'#g' | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)/ReadMe.rtf"
	echo '</installer-gui-script>' >> "$<.pkg/Distribution"
	pkgutil --flatten "$<.pkg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_LANG_PACK_VERSION).pkg"
# Sign package
	mv -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_LANG_PACK_VERSION).pkg" "$(INSTALL_HOME)/unsigned_$(@:build.package_%=%).pkg"
	productsign --sign "$(CERTPKGIDENTITY)" "$(INSTALL_HOME)/unsigned_$(@:build.package_%=%).pkg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_LANG_PACK_VERSION).pkg"
	rm -f "$(INSTALL_HOME)/unsigned_$(@:build.package_%=%).pkg"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	rm -Rf "$(INSTALL_HOME)/tmp"
	mkdir -p "$(INSTALL_HOME)/tmp"
	cd "$(INSTALL_HOME)/tmp" ; unzip "$(PWD)/etc/package/SetFileIcon.zip" ; $(CC) -o "SetFileIcon/SetFileIcon" -framework AppKit "SetFileIcon/SetFileIcon.m" ; "SetFileIcon/SetFileIcon" -image "$(PWD)/etc/package/ship.icns" -file "$(PWD)/$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_LANG_PACK_VERSION).pkg"
	rm -Rf "$(INSTALL_HOME)/tmp"
endif
	chmod -Rf a-w,a+r "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)"
	chmod -f u+w "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)"
# Use YourSway fancy .dmg tool to create .dmg file
	rm -Rf "$(INSTALL_HOME)/tmp"
	mkdir -p "$(INSTALL_HOME)/tmp"
	cd "$(INSTALL_HOME)/tmp" ; unzip "$(PWD)/etc/package/$(YOURSWAYCREATEDMG_SOURCE_FILENAME)"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	sync ; "$(INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" --volname "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_LANG_PACK_VERSION)" --volicon "etc/package/ship.icns" --icon-size 128 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_LANG_PACK_VERSION).pkg" 150 100 --icon "ReadMe.rtf" 350 100 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_LANG_PACK_VERSION).pkg" 150 100 --icon "ReadMe.rtf" 350 100 --window-pos 400 300 --window-size 500 250 "$(INSTALL_HOME)/$(PRODUCT_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).dmg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)"
else
	sync ; "$(INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).dmg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)"
endif
	rm -Rf "$(INSTALL_HOME)/tmp"

build.source_zip:
	"$(MAKE)" $(MFLAGS) "build.source_zip_shared"
	touch "$@"

build.source_zip_shared:
	rm -Rf "$(SOURCE_HOME)"
	mkdir -p "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	cd "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" ; cvs -d "$(NEO_CVSROOT)" co $(NEO_TAG) "$(NEO_PACKAGE)"
# Prune out empty directories
	cd "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" ; sh -e -c 'for i in `ls -1`; do cd "$${i}" ; cvs update -P -d ; done'
	cp "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/neojava/etc/gpl.html" "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/LICENSE.html"
	chmod -Rf u+w,og-w,a+r "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	cd "$(SOURCE_HOME)" ; gnutar zcf "$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION).src.tar.gz" "$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"

build.all: build.package build.package2 build.package3
	touch "$@"

build.all_patches: build.patch_package build.patch_package2 build.patch_package3
	touch "$@"
