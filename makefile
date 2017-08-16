#######################################################################
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
TARGET_MACHINE=x86_64
TARGET_FILE_TYPE=Mach-O 64-bit executable $(TARGET_MACHINE)
SHARED_LIBRARY_FILE_TYPE=Mach-O 64-bit dynamically linked shared library $(TARGET_MACHINE)
BUNDLE_FILE_TYPE=Mach-O 64-bit bundle $(TARGET_MACHINE)
else
OS_TYPE=Win32
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
LIBO_PACKAGE=libreoffice-$(LIBO_PRODUCT_VERSION)
LIBO_SOURCE_FILE=$(LIBO_PACKAGE).tar.xz
LIBO_BUILD_HOME=$(BUILD_HOME)/$(LIBO_PACKAGE)
LIBO_BOOTSTRAP_MAKEFILE:=$(BUILD_HOME)/bootstrap.mk
LIBO_INSTDIR=$(LIBO_BUILD_HOME)/instdir
LIBO_WORKDIR=$(LIBO_BUILD_HOME)/workdir
LIBO_LANGUAGES:=ar cs da de el en-GB en-US es fi fr he hu it ja ko nb nl pl pt-BR ru sk sv th tr zh-CN zh-TW
NEOLIGHT_MDIMPORTER_ID:=org.neooffice.neolight
NEOPEEK_QLPLUGIN_ID:=org.neooffice.quicklookplugin
INSTDIR=$(BUILD_HOME)/instdir
WORKDIR=$(BUILD_HOME)/workdir

# Product information
LIBO_PRODUCT_VERSION_FAMILY=4
LIBO_PRODUCT_NAME=LibreOfficeDev
LIBO_PRODUCT_VERSION=$(LIBO_PRODUCT_VERSION_FAMILY).4.7.2
PRODUCT_INSTALL_DIR_NAME=$(PRODUCT_NAME)
PRODUCT_VERSION_FAMILY=4.0
PRODUCT_VERSION_BASE=2017
PRODUCT_VERSION=$(PRODUCT_VERSION_BASE)
PRODUCT_VERSION2=$(PRODUCT_VERSION) $(PRODUCT_VERSION_EXT2)
PRODUCT_VERSION3=$(PRODUCT_VERSION) $(PRODUCT_VERSION_EXT3)
PRODUCT_VERSION_EXT=
PRODUCT_VERSION_EXT2=Viewer
PRODUCT_VERSION_EXT3=Professional Edition
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
PRODUCT_MIN_OSVERSION=10.12
PRODUCT_MIN_OSVERSION_NAME=$(PRODUCT_MIN_OSVERSION) Sierra
PRODUCT_MAX_OSVERSION=10.13
PRODUCT_MAX_OSVERSION_NAME=$(PRODUCT_MAX_OSVERSION) High Sierra
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
PRODUCT_BUNDLED_LANG_PACKS=$(LIBO_LANGUAGES)
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
PRODUCT_USER_INSTALL_DIR=$$HOME/Library/Containers/$(PRODUCT_DOMAIN).$(PRODUCT_DIR_NAME)/Data/Library/Preferences/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)
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
NEO_TAG:=NeoOffice-2017
NEO_TAG2:=$(NEO_TAG)
NEO_TAG3:=$(NEO_TAG)
PRODUCT_MODULES = \
	unotest \
	sal \
	jvmfwk \
	vcl \
	apple_remote \
	avmedia \
	basic \
	canvas \
	connectivity \
	cppcanvas \
	cppu \
	cppuhelper \
	cpputools \
	cui \
	dbaccess \
	desktop \
	drawinglayer \
	editeng \
	extensions \
	filter \
	fpicker \
	framework \
	i18npool \
	i18nutil \
	lingucomponent \
	linguistic \
	oox \
	package \
	pyuno \
	reportdesign \
	salhelper \
	sc \
	sd \
	sdext \
	sfx2 \
	shell \
	slideshow \
	svl \
	svtools \
	svx \
	sw \
	tools \
	ucb \
	ucbhelper \
	unotools \
	uui \
	writerfilter \
	xmloff

.DELETE_ON_ERROR : build.neo_configure

all: build.all

# Include dependent makefiles
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

build.libo_external_tarballs_checkout: build.libo_src_checkout build.libo_download.lst_patch
	rm -Rf "$(LIBO_BUILD_HOME)/external/tarballs"
	mkdir -p "$(LIBO_BUILD_HOME)/external/tarballs"
	cd "$(LIBO_PATCHES_HOME)/external/tarballs" ; sh -c -e 'for i in `find . -type f -maxdepth 1 | grep -v /CVS/` ; do cp "$$i" "$(PWD)/$(LIBO_BUILD_HOME)/external/tarballs/$$i" ; done'
	touch "$@"

build.libo_external_patches_checkout: build.libo_src_checkout
	cd "$(LIBO_PATCHES_HOME)/external/hsqldb/patches" ; sh -c -e 'for i in `find . -type f -maxdepth 1 | grep -v /CVS/` ; do cp "$$i" "$(PWD)/$(LIBO_BUILD_HOME)/external/hsqldb/patches/$$i" ; done'
	touch "$@"

build.libo_checkout: \
	build.libo_src_checkout \
	build.ant_checkout \
	build.libo_external_tarballs_checkout \
	build.libo_external_patches_checkout
	touch "$@"

build.libo_patches: \
	build.libo_configure.ac_patch \
	build.libo_download.lst_patch \
	build.libo_avmedia_patch \
	build.libo_bin_patch \
	build.libo_embeddedobj_patch \
	build.libo_external_patch \
	build.libo_include_patch \
	build.libo_jvmfwk_patch \
	build.libo_sc_patch \
	build.libo_sd_patch \
	build.libo_svx_patch \
	build.libo_sw_patch \
	build.libo_vcl_patch \
	build.libo_xmlhelp_patch
	touch "$@"

build.libo_configure.ac_patch: $(LIBO_PATCHES_HOME)/configure.ac.patch build.libo_checkout
ifeq ("$(OS_TYPE)","macOS")
	-( cd "$(LIBO_BUILD_HOME)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(LIBO_BUILD_HOME)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
else
	-cat "$<" | unix2dos | ( cd "$(LIBO_BUILD_HOME)" ; patch -b -R -p0 -N -r "/dev/null" )
	cat "$<" | unix2dos | ( cd "$(LIBO_BUILD_HOME)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" )
endif
	touch "$@"

build.libo_download.lst_patch: $(LIBO_PATCHES_HOME)/download.lst.patch build.libo_src_checkout
ifeq ("$(OS_TYPE)","macOS")
	-( cd "$(LIBO_BUILD_HOME)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(LIBO_BUILD_HOME)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
else
	-cat "$<" | unix2dos | ( cd "$(LIBO_BUILD_HOME)" ; patch -b -R -p0 -N -r "/dev/null" )
	cat "$<" | unix2dos | ( cd "$(LIBO_BUILD_HOME)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" )
endif
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
	cd "$(LIBO_BUILD_HOME)" ; unset DYLD_LIBRARY_PATH ; PATH=/usr/bin:/bin:/usr/sbin:/sbin:/opt/local/bin ; export PATH ; autoconf ; ./configure --without-parallelism --with-ant-home="$(PWD)/$(BUILD_HOME)/$(ANT_PACKAGE)" --with-macosx-version-min-required="$(PRODUCT_MIN_OSVERSION)" --without-junit --disable-cups --disable-odk --with-lang="$(LIBO_LANGUAGES)" --without-fonts --with-help --with-myspell-dicts --enable-bogus-pkg-config
else
	@echo "$@ not implemented" ; exit 1
endif
	touch "$@"

build.libo_all: build.libo_configure
ifeq ("$(OS_TYPE)","macOS")
	cd "$(LIBO_BUILD_HOME)" ; unset DYLD_LIBRARY_PATH ; PATH=/usr/bin:/bin:/usr/sbin:/sbin:/opt/local/bin ; export PATH ; make
else
	@echo "$@ not implemented" ; exit 1
endif
	touch "$@"

build.neo_instdir : build.libo_all $(LIBO_INSTDIR)
	rm -Rf "$(INSTDIR)"
	mkdir -p "$(INSTDIR)"
	cd "$(INSTDIR)" ; sh -e -c '( cd "$(PWD)/$(LIBO_INSTDIR)" ; find . -type d | sed "s/ /\\ /g" ) | while read i ; do mkdir -p "$$i" ; done'
ifeq ("$(OS_TYPE)","macOS")
	cd "$(INSTDIR)" ; sh -e -c '( cd "$(PWD)/$(LIBO_INSTDIR)" ; find . ! -type d | sed "s/ /\\ /g" ) | while read i ; do if [ ! -f "$$i" ] ; then ln -sf "$(PWD)/$(LIBO_INSTDIR)/$$i" "$$i" 2>/dev/null ; fi ; done'
else
# Use hardlinks for Windows
	cd "$(INSTDIR)" ; sh -e -c 'CYGWIN=winsymlinks ; export CYGWIN ; ( cd "$(PWD)/$(LIBO_INSTDIR)" ; find . ! -type d | sed "s/ /\\ /g" ) | while read i ; do if [ ! -f "$$i" ] ; then ln -f "$(PWD)/$(LIBO_INSTDIR)/$$i" "$$i" 2>/dev/null ; fi ; done'
endif
	touch "$@"

build.neo_workdir : build.libo_all $(LIBO_WORKDIR)
	rm -Rf "$(WORKDIR)"
	mkdir -p "$(WORKDIR)"
	cd "$(WORKDIR)" ; sh -e -c '( cd "$(PWD)/$(LIBO_WORKDIR)" ; find . -type d | sed "s/ /\\ /g" ) | while read i ; do mkdir -p "$$i" ; done'
ifeq ("$(OS_TYPE)","macOS")
	cd "$(WORKDIR)" ; sh -e -c '( cd "$(PWD)/$(LIBO_WORKDIR)" ; find . ! -type d | sed "s/ /\\ /g" ) | while read i ; do if [ ! -f "$$i" ] ; then ln -sf "$(PWD)/$(LIBO_WORKDIR)/$$i" "$$i" 2>/dev/null ; fi ; done'
else
# Use hardlinks for Windows
	cd "$(WORKDIR)" ; sh -e -c 'CYGWIN=winsymlinks ; export CYGWIN ; ( cd "$(PWD)/$(LIBO_WORKDIR)" ; find . ! -type d | sed "s/ /\\ /g" ) | while read i ; do if [ ! -f "$$i" ] ; then ln -f "$(PWD)/$(LIBO_WORKDIR)/$$i" "$$i" 2>/dev/null ; fi ; done'
endif
	touch "$@"

build.neo_configure: build.neo_instdir build.neo_workdir $(INSTDIR) $(WORKDIR)
	rm -f "$@"
	echo "# Modify some of LibreOffice's environment variables" >> "$@"
	echo "unexport DYLD_LIBRARY_PATH" >> "$@"
	echo "export PATH:=/usr/bin:/bin:/usr/sbin:/sbin:/opt/local/bin" >> "$@"
	echo "include "'$$(dir $$(realpath $$(firstword $$(MAKEFILE_LIST))))../$(LIBO_BUILD_HOME)/config_host.mk' >> "$@"
	echo "# Save LibreOffice environment variables" >> "$@"
	echo "export LIBO_SRCDIR:="'$$(SRCDIR)' >> "$@"
	echo "# Override LibreOffice environment variables" >> "$@"
	echo "export GUIBASE:=java" >> "$@"
	echo "export INSTDIR:=$(realpath $(INSTDIR))" >> "$@"
	echo "export INSTDIR_FOR_BUILD:=$(realpath $(INSTDIR))" >> "$@"
	echo "export INSTROOT:=$(realpath $(INSTDIR))"'/$$(PRODUCTNAME).app/Contents' >> "$@"
	echo "export INSTROOT_FOR_BUILD:=$(realpath $(INSTDIR))"'/$$(PRODUCTNAME).app/Contents' >> "$@"
	echo "export SOLARINC:=-I. -I$(realpath include)"' $$(SOLARINC)' >> "$@"
	echo "export SRC_ROOT:=$(realpath .)" >> "$@"
	echo "export SRCDIR:=$(realpath .)" >> "$@"
	echo "export WORKDIR:=$(realpath $(WORKDIR))" >> "$@"
	echo "export WORKDIR_FOR_BUILD:=$(realpath $(WORKDIR))" >> "$@"
	echo "# Add custom environment variables" >> "$@"
	echo "export PRODUCT_BUILD_TYPE:=java" >> "$@"
	echo "export PRODUCT_NAME:=$(PRODUCT_NAME)" >> "$@"
	echo "export PRODUCT_DIR_NAME:=$(PRODUCT_DIR_NAME)" >> "$@"
	echo "export PRODUCT_DIR_NAME2:=$(PRODUCT_DIR_NAME2)" >> "$@"
	echo "export PRODUCT_DIR_NAME3:=$(PRODUCT_DIR_NAME3)" >> "$@"
	echo "export PRODUCT_DOMAIN:=$(PRODUCT_DOMAIN)" >> "$@"
	echo "export PRODUCT_FILETYPE:=$(PRODUCT_FILETYPE)" >> "$@"
	echo "export PRODUCT_MAC_APP_STORE_URL:=$(PRODUCT_MAC_APP_STORE_URL)" >> "$@"
	echo "export PRODUCT_JAVA_DOWNLOAD_URL:=$(PRODUCT_JAVA_DOWNLOAD_URL)" >> "$@"
	echo "export PRODUCT_MIN_OSVERSION:=$(PRODUCT_MIN_OSVERSION)" >> "$@"
	echo "export PRODUCT_MAX_OSVERSION:=$(PRODUCT_MAX_OSVERSION)" >> "$@"

build.neo_patches: \
	$(PRODUCT_COMPONENT_MODULES:%=build.neo_%_component) \
	$(PRODUCT_COMPONENT_PATCH_MODULES:%=build.neo_%_component) \
	$(PRODUCT_MODULES:%=build.neo_%_patch)
	touch "$@"

# Custom modules that need to link directly to other custom modules
build.neo_sfx2_patch: build.neo_sal_patch build.neo_unotools_patch
build.neo_svtools_patch: build.neo_extensions_patch build.neo_vcl_patch

build.neo_solenv_patch: solenv build.neo_configure
	cd "$<" ; sh -e -c '( cd "$(PWD)/$(LIBO_BUILD_HOME)/$<" ; find . -type d | sed "s/ /\\ /g" | grep -v /CVS$$ ) | while read i ; do mkdir -p "$$i" ; done'
ifeq ("$(OS_TYPE)","macOS")
	cd "$<" ; sh -e -c '( cd "$(PWD)/$(LIBO_BUILD_HOME)/$<" ; find . ! -type d | sed "s/ /\\ /g" | grep -v /CVS/ ) | while read i ; do if [ ! -f "$$i" ] ; then ln -sf "$(PWD)/$(LIBO_BUILD_HOME)/$</$$i" "$$i" 2>/dev/null ; fi ; done'
else
# Use hardlinks for Windows
	cd "$<" ; sh -e -c 'CYGWIN=winsymlinks ; export CYGWIN ; ( cd "$(PWD)/$(LIBO_BUILD_HOME)/$<" ; find . ! -type d | grep -v /CVS/ ) | while read i ; do if [ ! -f "$$i" ] ; then ln -f "$(PWD)/$(LIBO_BUILD_HOME)/$</$$i" "$$i" 2>/dev/null ; fi ; done'
endif
	touch "$@"

build.neo_%_patch: build.neo_solenv_patch
	cd "$(@:build.neo_%_patch=%)" ; sh -e -c '( cd "$(PWD)/$(LIBO_BUILD_HOME)/$(@:build.neo_%_patch=%)" ; find . -type d | sed "s/ /\\ /g" | grep -v /CVS$$ ) | while read i ; do mkdir -p "$$i" ; done'
ifeq ("$(OS_TYPE)","macOS")
	cd "$(@:build.neo_%_patch=%)" ; sh -e -c '( cd "$(PWD)/$(LIBO_BUILD_HOME)/$(@:build.neo_%_patch=%)" ; find . ! -type d | sed "s/ /\\ /g" | grep -v /CVS/ ) | while read i ; do if [ ! -f "$$i" ] ; then ln -sf "$(PWD)/$(LIBO_BUILD_HOME)/$(@:build.neo_%_patch=%)/$$i" "$$i" 2>/dev/null ; fi ; done'
else
# Use hardlinks for Windows
	cd "$(@:build.neo_%_patch=%)" ; sh -e -c 'CYGWIN=winsymlinks ; export CYGWIN ; ( cd "$(PWD)/$(LIBO_BUILD_HOME)/$(@:build.neo_%_patch=%)" ; find . ! -type d | grep -v /CVS/ ) | while read i ; do if [ ! -f "$$i" ] ; then ln -f "$(PWD)/$(LIBO_BUILD_HOME)/$(@:build.neo_%_patch=%)/$$i" "$$i" 2>/dev/null ; fi ; done'
endif
	cd "$(@:build.neo_%_patch=%)" ; $(MAKE) $(MFLAGS) clean
	cd "$(@:build.neo_%_patch=%)" ; $(MAKE) $(MFLAGS)
	touch "$@"

build.neo_%_component: build.neo_solenv_patch
	cd "$(@:build.neo_%_component=%)" ; $(MAKE) $(MFLAGS) clean
	cd "$(@:build.neo_%_component=%)" ; $(MAKE) $(MFLAGS)
	touch "$@"

build.neo_tests: $(PRODUCT_MODULES:%=build.neo_%_test)
	touch "$@"

build.neo_%_test: build.neo_patches
	cd "$(@:build.neo_%_test=%)" ; $(MAKE) $(MFLAGS) check
	touch "$@"

build.check_env_vars: build.neo_configure
	@"$(MAKE)" $(MFLAGS) PARENT_PRODUCT_NAME="$(PRODUCT_NAME)" PARENT_PRODUCT_DOMAIN="$(PRODUCT_DOMAIN)" PARENT_PRODUCT_DIR_NAME="$(PRODUCT_DIR_NAME)" PARENT_PRODUCT_DIR_NAME2="$(PRODUCT_DIR_NAME2)" PARENT_PRODUCT_DIR_NAME3="$(PRODUCT_DIR_NAME3)" PARENT_PRODUCT_MAC_APP_STORE_URL="$(PRODUCT_MAC_APP_STORE_URL)" PARENT_PRODUCT_JAVA_DOWNLOAD_URL="$(PRODUCT_JAVA_DOWNLOAD_URL)" PARENT_PRODUCT_MIN_OSVERSION="$(PRODUCT_MIN_OSVERSION)" PARENT_PRODUCT_MAX_OSVERSION="$(PRODUCT_MAX_OSVERSION)" -f "$(PWD)/etc/Makefile" check_env_vars

build.package: build.neo_tests
ifeq ("$(PRODUCT_PATCH_VERSION)","Patch 0")
	"$(MAKE)" $(MFLAGS) PRODUCT_PATCH_VERSION=" " "build.package"
else
	"$(MAKE)" $(MFLAGS) "build.check_env_vars"
	"$(MAKE)" $(MFLAGS) "build.package_shared"
	touch "$@"
endif

build.package2: build.neo_tests
ifndef PRODUCT_BUILD2
ifeq ("$(PRODUCT_PATCH_VERSION)","Patch 0")
	"$(MAKE)" $(MFLAGS) PRODUCT_PATCH_VERSION=" " "build.package2"
else
	"$(MAKE)" $(MFLAGS) "build.check_env_vars"
	"$(MAKE)" $(MFLAGS) "PRODUCT_BUILD2=TRUE" "PRODUCT_VERSION=$(PRODUCT_VERSION2)" "PRODUCT_VERSION_EXT=$(PRODUCT_VERSION_EXT2)" "PRODUCT_DIR_NAME=$(PRODUCT_DIR_NAME2)" "CERTAPPIDENTITY=$(CERTAPPIDENTITY2)" "CERTPKGIDENTITY=$(CERTPKGIDENTITY2)" "PRODUCT_BUNDLED_LANG_PACKS=$(PRODUCT_BUNDLED_LANG_PACKS2)" "build.package_shared"
	touch "$@"
endif
endif

build.package3: build.neo_tests
ifndef PRODUCT_BUILD3
ifeq ("$(PRODUCT_PATCH_VERSION)","Patch 0")
	"$(MAKE)" $(MFLAGS) PRODUCT_PATCH_VERSION=" " "build.package3"
else
	"$(MAKE)" $(MFLAGS) "build.check_env_vars"
	"$(MAKE)" $(MFLAGS) "PRODUCT_BUILD3=TRUE" "PRODUCT_VERSION=$(PRODUCT_VERSION3)" "PRODUCT_VERSION_EXT=$(PRODUCT_VERSION_EXT3)" "PRODUCT_DIR_NAME=$(PRODUCT_DIR_NAME3)" "CERTAPPIDENTITY=$(CERTAPPIDENTITY3)" "CERTPKGIDENTITY=$(CERTPKGIDENTITY3)" "PRODUCT_BUNDLED_LANG_PACKS=$(PRODUCT_BUNDLED_LANG_PACKS3)" "build.package_shared"
	touch "$@"
endif
endif

build.package_shared:
# Check that codesign and productsign executables exist before proceeding
	@sh -e -c 'for i in codesign productsign ; do if [ -z "`which $$i`" ] ; then echo "$$i command not found" ; exit 1 ; fi ; done'
	sh -e -c 'if [ -d "$(INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(INSTALL_HOME)" ; fi'
	mkdir -p "$(INSTALL_HOME)/package"
	sh -e -c '( cd "$(LIBO_INSTDIR)/$(LIBO_PRODUCT_NAME).app" && tar cf - . ) | ( cd "$(PWD)/$(INSTALL_HOME)/package" && tar xvf - )'
	sh -e -c '( cd "$(INSTDIR)/$(LIBO_PRODUCT_NAME).app" && ( find . -type f | tar cf - -T - ) ) | ( cd "$(PWD)/$(INSTALL_HOME)/package" && tar xvf - )'
# Remove LibO system plugins. Also remove bundled dictionaries and any other
# extensions as it forces a restart on first run.
	rm -Rf "$(INSTALL_HOME)/package/Contents/Library"
	rm -Rf "$(INSTALL_HOME)/package/Contents/Resources/extensions"
	rm -Rf "$(INSTALL_HOME)/package/Contents/Resources/uno_packages"
	rm -Rf "$(INSTALL_HOME)/package/Contents/share/uno_packages"
	rm -Rf "$(INSTALL_HOME)/package/Contents/user"
	mkdir -p "$(INSTALL_HOME)/package/Contents/Resources/extensions"
# Eliminate sandbox deny file-write-create messages by recreating the cache
# subdirectory in uno_packages
	mkdir -p "$(INSTALL_HOME)/package/Contents/Resources/uno_packages/cache"
	chmod -Rf u+w,a+r "$(INSTALL_HOME)/package"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program" ; mv -f "MacOS" "program" ; mkdir -p "MacOS"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/soffice"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/soffice2"
	cd "$(INSTALL_HOME)/package/Contents" ; mv "program/soffice3" "MacOS/soffice.bin"
	cd "$(INSTALL_HOME)/package/Contents" ; mv "program/updateruninstallers" "MacOS/updchkruninstallers.bin"
else ifdef PRODUCT_BUILD2
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/soffice"
	cd "$(INSTALL_HOME)/package/Contents" ; mv "program/soffice2" "MacOS/soffice.bin"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/soffice3"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/updateruninstallers"
else
	cd "$(INSTALL_HOME)/package/Contents" ; mv "program/soffice" "MacOS/soffice.bin"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/soffice2"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/soffice3"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/updateruninstallers"
endif
# Remove unnecessary executables and files
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf CREDITS* LICENSE* NOTICE* Resources/oasis-*.icns program/gengal program/gengal.bin program/python program/regmerge program/regview program/senddoc program/ui-previewer program/unoinfo program/unopkg program/unpack_update
# Add executable softlinks
	cd "$(INSTALL_HOME)/package/Contents" ; ln -sf "soffice.bin" "MacOS/soffice"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/unopkg"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; ln -sf "soffice.bin" "MacOS/unopkg"
	cd "$(INSTALL_HOME)/package/Contents" ; ln -sf "soffice.bin" "MacOS/unopkg.bin"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd program && ls` ; do ln -sf "../program/$$i" "MacOS/$$i" ; done'
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
	rm -f "$(INSTALL_HOME)/package/Contents/Resources/config/images_tango_testing.zip"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/neo2toolbarv101.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_tango.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_tango.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_crystal.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_galaxy.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_galaxy.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_hicontrast.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_oxygen.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_oxygen.zip" .
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
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_tango.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_tango.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_crystal.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_galaxy.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_galaxy.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_hicontrast.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_oxygen.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_oxygen.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/NeoOfficeAquaElements.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "Resources/flat_logo.svg" "Resources/intro.png" "Resources/shell/about.svg"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/package/intro_professional.png" "Resources/intro.png"
else ifdef PRODUCT_BUILD2
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/package/intro_free.png" "Resources/intro.png"
else
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/package/intro.png" "Resources/intro.png"
endif
endif
	cd "$(INSTALL_HOME)/package/Contents" ; cp "tmp/NeoOffice Aqua Elements 3/Contents/Resources/"*.icns "Resources"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/package/ship.icns" "Resources"
endif
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_tango.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_tango.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_crystal.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_galaxy.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_galaxy.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_hicontrast.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_oxygen.zip" ; find . -exec touch {} \; ; zip -ruD "$(PWD)/$(INSTALL_HOME)/package/Contents/Resources/config/images_oxygen.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; tidy -quiet -xml -utf8 -wrap 4096 --hide-comments 1 "$(PWD)/etc/Resources/services/services.rdb" | xmllint --noblanks - > "Resources/services/services.rdb"
else
	cd "$(INSTALL_HOME)/package/Contents" ; tidy -quiet -xml -utf8 -wrap 4096 --hide-comments 1 "$(PWD)/etc/sandbox/Resources/services/services.rdb" | xmllint --noblanks - > "Resources/services/services.rdb"
endif
# Add Mac OS X localized resources
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; sh -e -c 'for i in `echo "$(PRODUCT_BUNDLED_LANG_PACKS)" | sed "s#-#_#g"` ; do mkdir -p "$${i}.lproj" ; mkdir -p `echo "$${i}" | sed "s#_.*\\$$##"`".lproj" ; done'
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; ( ( cd "$(PWD)/etc/package/l10n" && gnutar cvf - --exclude CVS --exclude "*.html" . ) | gnutar xvf - )
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; sh -e -c 'for i in `cd "$(PWD)/etc/package/l10n" ; find . -name "*.html"` ; do sed "s#\$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g" "$(PWD)/etc/package/l10n/$${i}" | sed "s#\$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g" | sed "s#\$$(PRODUCT_DOWNLOAD_URL)#$(PRODUCT_DOWNLOAD_URL)#g" | sed "s#\$$(PRODUCT_MIN_OSVERSION_NAME)#$(PRODUCT_MIN_OSVERSION_NAME)#g" | sed "s#\$$(PRODUCT_MAX_OSVERSION_NAME)#$(PRODUCT_MAX_OSVERSION_NAME)#g" > "$${i}" ; done'
endif
# Remove LibO native spellchecker and all OpenGL libraries. Fix bug 3273 by not
# installing any LibO fonts. Remove Lotus Word Pro import library as it has
# several known security vulnerabilities. Remove OpenGL transitions.
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "Frameworks/libMacOSXSpelllo.dylib" "Frameworks/libOGLTranslo.dylib" "Frameworks/libavmediaMacAVFlo.dylib" "Frameworks/libavmediaogl.dylib" "Frameworks/liblwpftlo.dylib" "Frameworks/liboglcanvaslo.dylib" "Resources/config/soffice.cfg/simpress/transitions-ogl.xml" "Resources/fonts/truetype" "Resources/java/smoketest.jar"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find "Resources/wizards" -type f -not -path "*/CVS/*" | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find "Resources/wizards" -type f -name "*.py" -not -path "*/CVS/*"` ; do sed "s#\$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g" "$(PWD)/etc/$${i}" > "$${i}" ; done'
else
# Remove update check files since the Mac App Store has its own update
# check. Do not remove updatefeed.uno.dylib as it is needed by the pdfimport
# extension.
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "Frameworks/libupdchklo.dylib"
# Remove Java files
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "Frameworks/libhsqldb.jnilib" "Frameworks/libjava_uno.dylib" "Frameworks/libjava_uno.jnilib" "Frameworks/libjavaloaderlo.dylib" "Frameworks/libjavavmlo.dylib" "Frameworks/libjdbclo.dylib" "Frameworks/libjpipe.jnilib" "Frameworks/libjuh.jnilib" "Frameworks/libjuhx.dylib" "Resources/Scripts/beanshell" "Resources/Scripts/java" "Resources/Scripts/javascript" "Resources/config/soffice.cfg/modules/dbapp/toolbar/reportobjectbar.xml" "Resources/config/soffice.cfg/cui/ui/javaclasspathdialog.ui" "Resources/config/soffice.cfg/cui/ui/javastartparametersdialog.ui" "Resources/config/soffice.cfg/svt/ui/javadisableddialog.ui" "Resources/java" "Resources/registry/reportbuilder.xcd" "Resources/services/scriptproviderforbeanshell.rdb" "Resources/services/scriptproviderforjavascript.rdb" "Resources/ure/share/misc/javavendors.xml"
	cd "$(INSTALL_HOME)/package/Contents/Resources/help" ; sh -e -c 'for i in `find . -type d -name "*.idxl"` ; do rm -Rf "$${i}" ; done'
# Remove Python files
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "Frameworks/LibreOfficePython.framework" "Frameworks/libpythonloaderlo.dylib" "Frameworks/pyuno.so" "Frameworks/libpyuno.dylib" "Resources/Scripts/python" "Resources/mailmerge.py" "Resources/msgbox.py" "Resources/officehelper.py" "Resources/pythonloader.py" "Resources/pythonloader.unorc" "Resources/pythonscript.py" "Resources/registry/pyuno.xcd" "Resources/uno.py" "Resources/unohelper.py" "Resources/services/pyuno.rdb" "Resources/services/scriptproviderforpython.rdb" "Resources/wizards/__init__.py" "Resources/wizards/agenda" "Resources/wizards/common" "Resources/wizards/document" "Resources/wizards/fax" "Resources/wizards/letter" "Resources/wizards/text" "Resources/wizards/ui" "Resources/wizards/web"
endif
# Copy custom fonts
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find "Resources/fonts/truetype" -type f -not -path "*/CVS/*" | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find "Resources/fonts/truetype" -type f -name "*.ttf" -not -path "*/CVS/*"` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find "Resources/config" "Resources/registry" -type f -not -path "*/CVS/*" | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find "Resources/config" "Resources/registry" -type f -name "*.xcd" -not -path "*/CVS/*"` ; do tidy -quiet -xml -utf8 -wrap 4096 --hide-comments 1 "$(PWD)/etc/$${i}" | xmllint --noblanks - > "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find "Resources/config" "Resources/registry" -type f -name "*.xml" -not -path "*/CVS/*"` ; do tidy -quiet -xml -utf8 -wrap 4096 -indent --hide-comments 1 "$(PWD)/etc/$${i}" > "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find "Resources/config" "Resources/registry" -type f -not -name "*.xcd" -not -name "*.xml"  -not -path "*/CVS/*"` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
ifndef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc/sandbox" ; find "Resources/config" "Resources/registry" -type f -not -path "*/CVS/*" | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc/sandbox" ; find "Resources/config" "Resources/registry" -type f -name "*.xcd" -not -path "*/CVS/*"` ; do tidy -quiet -xml -utf8 -wrap 4096 --hide-comments 1 "$(PWD)/etc/sandbox/$${i}" | xmllint --noblanks - > "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc/sandbox" ; find "Resources/config" "Resources/registry" -type f -name "*.xml" -not -path "*/CVS/*"` ; do tidy -quiet -xml -utf8 -wrap 4096 -indent --hide-comments 1 "$(PWD)/etc/sandbox/$${i}" > "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc/sandbox" ; find "Resources/config" "Resources/registry" -type f -not -name "*.xcd" -not -name "*.xml" -not -path "*/CVS/*"` ; do cp "$(PWD)/etc/sandbox/$${i}" "$${i}" ; done'
endif
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/Resources/bootstraprc" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' > "Resources/bootstraprc"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/Resources/sofficerc" "Resources/sofficerc"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/Resources/versionrc" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_UPDATE_CHECK_URL)#$(PRODUCT_UPDATE_CHECK_URL)#g' | sed 's#$$(LIBO_PRODUCT_VERSION)#$(LIBO_PRODUCT_VERSION)#g' | sed 's# #%20#g' | sed 's#^BuildVersion=.*$$#BuildVersion=$(PRODUCT_PATCH_VERSION)#' > "Resources/versionrc"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "Resources/registry/main.xcd" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "../../out" ; mv -f "../../out" "Resources/registry/main.xcd"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/NOTICE" > "Resources/NOTICE"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(LIBO_PRODUCT_NAME)#$(LIBO_PRODUCT_NAME)#g' "$(PWD)/etc/Resources/help/main_transform.xsl" | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL)#$(PRODUCT_DOCUMENTATION_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL_TEXT)#$(PRODUCT_DOCUMENTATION_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#g' > "Resources/help/main_transform.xsl"
else
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/sandbox/NOTICE" > "Resources/NOTICE"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(LIBO_PRODUCT_NAME)#$(LIBO_PRODUCT_NAME)#g' "$(PWD)/etc/sandbox/Resources/help/main_transform.xsl" | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL)#$(PRODUCT_DOCUMENTATION_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL_TEXT)#$(PRODUCT_DOCUMENTATION_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#g' > "Resources/help/main_transform.xsl"
endif
	cd "$(INSTALL_HOME)/package" ; find . -type f -exec file {} \; > "$(PWD)/$(INSTALL_HOME)/filetypes.txt"
ifndef PRODUCT_BUILD3
# Mac App Store will reject apps with shell scripts
	sh -e -c 'if grep "script text executable" "$(INSTALL_HOME)/filetypes.txt" ; then exit 1 ; fi'
endif
# Check if any binaries are linked to local libraries
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `grep -e "$(TARGET_FILE_TYPE)" -e "$(SHARED_LIBRARY_FILE_TYPE)" -e "$(BUNDLE_FILE_TYPE)" "$(PWD)/$(INSTALL_HOME)/filetypes.txt" | sed "s#:.*\\$$##"` ; do if otool -L "$$i" | grep -q "\/local\/" ; then otool -L "$$i" ; exit 1 ; fi ; done'
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `grep -e "$(TARGET_FILE_TYPE)" -e "$(SHARED_LIBRARY_FILE_TYPE)" -e "$(BUNDLE_FILE_TYPE)" "$(PWD)/$(INSTALL_HOME)/filetypes.txt" | sed "s#:.*\\$$##"` ; do strip -S -x "$$i" ; done'
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
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type d -name "*.mdimporter"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type d -name "*.qlgenerator"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
# Sign "A" version of each framework
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find "Contents/Frameworks" -type d -name "A"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `grep -e "$(SHARED_LIBRARY_FILE_TYPE)" -e "$(BUNDLE_FILE_TYPE)" "$(PWD)/$(INSTALL_HOME)/filetypes.txt" | sed "s#:.*\\$$##"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `grep "$(TARGET_FILE_TYPE)" "$(PWD)/$(INSTALL_HOME)/filetypes.txt" | sed "s#:.*\\$$##" | grep -v "\/soffice\.bin"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" .
else
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `grep "$(TARGET_FILE_TYPE)" "$(PWD)/$(INSTALL_HOME)/filetypes.txt" | sed "s#:.*\\$$##" | grep -v "\/soffice\.bin"` ; do codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/etc/package/Entitlements_inherit_only.plist" "$$i" ; done'
	cat "etc/package/Entitlements.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(INSTALL_HOME)/Entitlements.plist"
	cd "$(INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/$(INSTALL_HOME)/Entitlements.plist" .
endif
# Test that all libraries will load
	cd "$(INSTALL_HOME)/package" ; unset DYLD_LIBRARY_PATH ; PATH=/usr/bin:/bin:/usr/sbin:/sbin:/opt/local/bin ; export PATH ; $(CC) -arch "$(TARGET_MACHINE)" -o "Contents/MacOS/loaddyliblist" "$(PWD)/etc/package/loaddyliblist.c" ; sh -e -c 'grep -e "$(SHARED_LIBRARY_FILE_TYPE)" -e "$(BUNDLE_FILE_TYPE)" "$(PWD)/$(INSTALL_HOME)/filetypes.txt" | sed "s#:.*\$$##" | "Contents/MacOS/loaddyliblist" ; rm -f "Contents/MacOS/loaddyliblist"'
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
	sync ; "$(INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" --volname "Install $(PRODUCT_NAME) $(PRODUCT_VERSION)" --volicon "etc/package/ship.icns" --icon-size 128 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION).pkg" 150 100 --icon "ReadMe.rtf" 350 100 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION).pkg" 150 100 --icon "ReadMe.rtf" 350 100 --window-pos 400 300 --window-size 500 260 "$(INSTALL_HOME)/$(PRODUCT_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).dmg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
else
	sync ; "$(INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).dmg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
endif
	rm -Rf "$(INSTALL_HOME)/tmp"

build.patch_package: build.package
	"$(MAKE)" $(MFLAGS) "build.check_env_vars"
	"$(MAKE)" $(MFLAGS) "CERTAPPIDENTITY=$(PATCHCERTAPPIDENTITY)" "CERTPKGIDENTITY=$(PATCHCERTPKGIDENTITY)" "build.patch_package_shared"
	touch "$@"

build.patch_package2: build.package2
ifndef PRODUCT_BUILD2
	"$(MAKE)" $(MFLAGS) "build.check_env_vars"
	"$(MAKE)" $(MFLAGS) "PRODUCT_BUILD2=TRUE" "PRODUCT_VERSION=$(PRODUCT_VERSION2)" "PRODUCT_VERSION_EXT=$(PRODUCT_VERSION_EXT2)" "PRODUCT_DIR_NAME=$(PRODUCT_DIR_NAME2)" "CERTAPPIDENTITY=$(PATCHCERTAPPIDENTITY2)" "CERTPKGIDENTITY=$(PATCHCERTPKGIDENTITY2)" "PRODUCT_BUNDLED_LANG_PACKS=$(PRODUCT_BUNDLED_LANG_PACKS2)" "build.patch_package_shared"
	touch "$@"
endif

build.patch_package3: build.package3
ifndef PRODUCT_BUILD3
	"$(MAKE)" $(MFLAGS) "build.check_env_vars"
	"$(MAKE)" $(MFLAGS) "PRODUCT_BUILD3=TRUE" "PRODUCT_VERSION=$(PRODUCT_VERSION3)" "PRODUCT_VERSION_EXT=$(PRODUCT_VERSION_EXT3)" "PRODUCT_DIR_NAME=$(PRODUCT_DIR_NAME3)" "CERTAPPIDENTITY=$(PATCHCERTAPPIDENTITY3)" "CERTPKGIDENTITY=$(PATCHCERTPKGIDENTITY3)" "PRODUCT_BUNDLED_LANG_PACKS=$(PRODUCT_BUNDLED_LANG_PACKS3)" "build.patch_package_shared"
	touch "$@"
endif

build.patch_package_shared:
# Check that codesign and productsign executables exist before proceeding
	@sh -e -c 'for i in codesign productsign ; do if [ -z "`which $$i`" ] ; then echo "$$i command not found" ; exit 1 ; fi ; done'
	sh -e -c 'if [ -d "$(PATCH_INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(PATCH_INSTALL_HOME)" ; fi'
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/Frameworks"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/MacOS"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/Resources"
ifdef PRODUCT_BUILD3
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(INSTDIR)/$(LIBO_PRODUCT_NAME).app/Contents/MacOS/soffice3" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
else ifdef PRODUCT_BUILD2
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(INSTDIR)/$(LIBO_PRODUCT_NAME).app/Contents/MacOS/soffice2" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
else
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(INSTDIR)/$(LIBO_PRODUCT_NAME).app/Contents/MacOS/soffice" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
endif
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(INSTDIR)/$(LIBO_PRODUCT_NAME).app/Contents/Frameworks/libvcllo.dylib" "Frameworks"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/package/Info.plist" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME)#$(PRODUCT_TRADEMARKED_NAME)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(PRODUCT_FILETYPE)#$(PRODUCT_FILETYPE)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(PRODUCT_BUILD_VERSION)#$(PRODUCT_VERSION_BASE)#g' > "Info.plist"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/Resources/bootstraprc" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' > "Resources/bootstraprc"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/Resources/versionrc" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_UPDATE_CHECK_URL)#$(PRODUCT_UPDATE_CHECK_URL)#g' | sed 's#$$(LIBO_PRODUCT_VERSION)#$(LIBO_PRODUCT_VERSION)#g' | sed 's# #%20#g' | sed 's#^BuildVersion=.*$$#BuildVersion=$(PRODUCT_PATCH_VERSION)#' > "Resources/versionrc"
	cd "$(PATCH_INSTALL_HOME)/package" ; find . -type f -exec file {} \; > "$(PWD)/$(PATCH_INSTALL_HOME)/filetypes.txt"
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `grep -e "$(TARGET_FILE_TYPE)" -e "$(SHARED_LIBRARY_FILE_TYPE)" -e "$(BUNDLE_FILE_TYPE)" "$(PWD)/$(PATCH_INSTALL_HOME)/filetypes.txt" | sed "s#:.*\\$$##"` ; do strip -S -x "$$i" ; done'
	xattr -rcs "$(PATCH_INSTALL_HOME)/package"
# Sign all binaries
	chmod -Rf u+rw "$(PATCH_INSTALL_HOME)/package"
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type d -name "*.mdimporter"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type d -name "*.qlgenerator"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
# Sign "A" version of each framework
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find "Contents/Frameworks" -type d -name "A"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `grep -e "$(SHARED_LIBRARY_FILE_TYPE)" -e "$(BUNDLE_FILE_TYPE)" "$(PWD)/$(PATCH_INSTALL_HOME)/filetypes.txt" | sed "s#:.*\\$$##"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
ifdef PRODUCT_BUILD3
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `grep "$(TARGET_FILE_TYPE)" "$(PWD)/$(PATCH_INSTALL_HOME)/filetypes.txt" | sed "s#:.*\\$$##" | grep -v "\/soffice\.bin"` ; do codesign --force -s "$(CERTAPPIDENTITY)" "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" .
else
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `grep "$(TARGET_FILE_TYPE)" "$(PWD)/$(PATCH_INSTALL_HOME)/filetypes.txt" | sed "s#:.*\\$$##" | grep -v "\/soffice\.bin"` ; do codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/etc/package/Entitlements_inherit_only.plist" "$$i" ; done'
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
	sync ; "$(PATCH_INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" --volname "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION)" --volicon "etc/package/ship.icns" --icon-size 128 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION).pkg" 250 100 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION).pkg" 250 100 --window-pos 400 300 --window-size 500 260 "$(PATCH_INSTALL_HOME)/$(PRODUCT_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).dmg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
else
	sync ; "$(PATCH_INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).dmg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
endif
	rm -Rf "$(PATCH_INSTALL_HOME)/tmp"

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
