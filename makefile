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

# Set the shell to tcsh since the OpenOffice.org build requires it
EMPTY:=
SPACE:=$(EMPTY) $(EMPTY)
ifndef TMP
TMP:=/tmp
endif
SHELL:=/bin/tcsh
UNAME:=$(shell uname -p)
ifeq ("$(shell uname -s)","Darwin")
OS_TYPE=MacOSX
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
MOZILLA_PATCHES_HOME:=patches/mozilla
MSWEET_PATCHES_HOME:=patches/msweet
NEOOFFICE_PATCHES_HOME:=patches/neooffice
OO_PATCHES_HOME:=patches/openoffice
OO_PACKAGE=aoo-4.1.2
OO_SOURCE_FILE=apache-openoffice-4.1.2-r1709696-src.tar.gz
OO_BUILD_HOME=$(BUILD_HOME)/$(OO_PACKAGE)/main
OOO-BUILD_PATCHES_HOME:=patches/ooo-build/src
REMOTECONTROL_PATCHES_HOME:=patches/remotecontrol
ifeq ("$(OS_TYPE)","MacOSX")
OO_ENV_AQUA:=$(OO_BUILD_HOME)/MacOSXX64Env.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacOSXX64EnvJava.Set
else
OO_ENV_AQUA:=$(OO_BUILD_HOME)/winenv.set
OO_ENV_JAVA:=$(BUILD_HOME)/winenv.set
endif
OO_LANGUAGES:=$(shell cat '$(PWD)/etc/supportedlanguages.txt' | sed '/^\#.*$$/d' | sed 's/\#.*$$//' | awk -F, '{ print $$1 }')
NEOLIGHT_MDIMPORTER_ID:=org.neooffice.neolight
NEOPEEK_QLPLUGIN_ID:=org.neooffice.quicklookplugin

# Product information
OO_PRODUCT_VERSION_FAMILY=3.1
OO_PRODUCT_NAME=OpenOffice.org
OO_PRODUCT_VERSION=3.1.1
OO_REGISTRATION_URL=http://survey.services.openoffice.org/user/index.php
PRODUCT_INSTALL_DIR_NAME=$(PRODUCT_NAME)
PRODUCT_VERSION_FAMILY=3.0
PRODUCT_VERSION_BASE=2015
PRODUCT_VERSION=$(PRODUCT_VERSION_BASE).4
PRODUCT_VERSION2=$(PRODUCT_VERSION) $(PRODUCT_VERSION_EXT2)
PRODUCT_VERSION3=$(PRODUCT_VERSION) $(PRODUCT_VERSION_EXT3)
PRODUCT_VERSION_EXT=
PRODUCT_VERSION_EXT2=Viewer
PRODUCT_VERSION_EXT3=Classic Edition
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
PRODUCT_MIN_OSVERSION=10.9
PRODUCT_MAX_OSVERSION=10.12
PRODUCT_FILETYPE=NO%F
PRODUCT_BASE_URL=http://www.neooffice.org/neojava
PRODUCT_REGISTRATION_URL=http://trinity.neooffice.org/modules.php?name=Your_Account\&amp\;redirect=index
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
ifeq ("$(OS_TYPE)","MacOSX")
PRODUCT_COMPONENT_MODULES=imagecapture remotecontrol
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
DMAKE_SOURCE_FILENAME=dmake-4.12.tar.bz2
EPM_PACKAGE=epm-3.7
EPM_SOURCE_FILENAME=epm-3.7-source.tar.gz
JFREEREPORT_PACKAGE=ooo310-m19-extensions
JFREEREPORT_SOURCE_FILENAME=ooo310-m19-extensions.tar.bz2
OO_EXTENSIONS_PACKAGE=OOO330_m20
OO_EXTENSIONS_SOURCE_FILENAME=OOo_3.3.0_src_extensions.tar.bz2
REMOTECONTROL_PACKAGE=martinkahr-apple_remote_control-2ba0484
REMOTECONTROL_SOURCE_FILENAME=martinkahr-apple_remote_control.tar.gz
YOURSWAYCREATEDMG_PACKAGE=jaeggir-yoursway-create-dmg-a22ac11
YOURSWAYCREATEDMG_SOURCE_FILENAME=yoursway-create-dmg.zip
NEO_CVSROOT:=:pserver:anoncvs@anoncvs.neooffice.org:/cvs
NEO_PACKAGE:=NeoOffice
NEO_TAG:=NeoOffice-2015_4
NEO_TAG2:=NeoOffice-2015_4_Viewer
NEO_TAG3:=NeoOffice-2015_4_Classic_Edition

all: build.all

# Include dependent makefiles
include neo_configure.mk

build.oo_src_checkout: $(OO_PATCHES_HOME)/$(OO_SOURCE_FILE)
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; tar zxvf "$(PWD)/$<"
	cd "$(BUILD_HOME)" ; chmod -Rf u+rw "$(OO_PACKAGE)"
	touch "$@"

build.ant_checkout: $(APACHE_PATCHES_HOME)/$(ANT_SOURCE_FILENAME)
	rm -Rf "$(BUILD_HOME)/$(ANT_PACKAGE)"
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; tar zxvf "$(PWD)/$(APACHE_PATCHES_HOME)/$(ANT_SOURCE_FILENAME)"
	touch "$@"

build.jfreereport_checkout: build.oo_src_checkout $(OO_PATCHES_HOME)/$(OO_EXTENSIONS_SOURCE_FILENAME) $(OOO-BUILD_PATCHES_HOME)/$(JFREEREPORT_SOURCE_FILENAME)
	rm -Rf "$(OO_BUILD_HOME)/jfreereport"
	mkdir -p "$(OO_BUILD_HOME)"
	cd "$(OO_BUILD_HOME)" ; bunzip2 -dc "$(PWD)/$(OO_PATCHES_HOME)/$(OO_EXTENSIONS_SOURCE_FILENAME)" | tar xvf - --strip-components=1 "$(OO_EXTENSIONS_PACKAGE)/jfreereport"
	cd "$(OO_BUILD_HOME)" ; bunzip2 -dc "$(PWD)/$(OOO-BUILD_PATCHES_HOME)/$(JFREEREPORT_SOURCE_FILENAME)" | tar xvf - --strip-components=1 "$(JFREEREPORT_PACKAGE)/jfreereport/download"
	touch "$@"

build.oo_ext_sources_checkout: build.jfreereport_checkout
	rm -Rf "$(OO_BUILD_HOME)/../ext_sources"
	mkdir -p "$(OO_BUILD_HOME)/../ext_sources"
	cd "$(OO_PATCHES_HOME)/ext_sources" ; sh -c -e 'for i in `find . -type f -maxdepth 1 | grep -v /CVS/` ; do cp "$$i" "$(PWD)/$(OO_BUILD_HOME)/../ext_sources/$$i" ; done'
	cd "$(APACHE_PATCHES_HOME)" ; sh -c -e 'for i in `find . -type f -maxdepth 1 | grep -v /CVS/` ; do cp "$$i" "$(PWD)/$(OO_BUILD_HOME)/../ext_sources/$$i" ; done'
# OOo perl script renames the dmake and epm source filenames incorrectly so
# copy them with the correct name
	cd "$(APACHE_PATCHES_HOME)" ; sh -c -e 'filename=`md5 -q "$(DMAKE_SOURCE_FILENAME)"`-"$(DMAKE_SOURCE_FILENAME)" ; cp "$(DMAKE_SOURCE_FILENAME)" "$(PWD)/$(OO_BUILD_HOME)/../ext_sources/$$filename"'
	cd "$(MSWEET_PATCHES_HOME)" ; sh -c -e 'filename=`md5 -q "$(EPM_SOURCE_FILENAME)"`-"$(EPM_PACKAGE).tar.gz" ; cp "$(EPM_SOURCE_FILENAME)" "$(PWD)/$(OO_BUILD_HOME)/../ext_sources/$$filename"'
	cd "$(OO_BUILD_HOME)/jfreereport/download" ; sh -c -e 'for i in `find . -name "*.zip" -maxdepth 1` ; do filename=`md5 -q "$$i"`-`basename "$$i"` ; cp "$$i" "$(PWD)/$(OO_BUILD_HOME)/../ext_sources/$$filename" ; done'
	touch "$@"

build.oo_checkout: build.oo_src_checkout build.ant_checkout build.jfreereport_checkout build.oo_ext_sources_checkout
	touch "$@"

build.oo_patches: \
	build.oo_configure.in_patch \
	build.oo_framework_patch \
	build.oo_jfreereport_patch \
	build.oo_reportbuilder_patch \
	build.oo_sal_patch \
	build.oo_sdext_patch \
	build.oo_solenv_patch
	touch "$@"

build.oo_%.in_patch: $(OO_PATCHES_HOME)/%.in.patch build.oo_checkout
	-( cd "$(OO_BUILD_HOME)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(OO_BUILD_HOME)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.oo_%_patch: $(OO_PATCHES_HOME)/%.patch build.oo_checkout
ifeq ("$(OS_TYPE)","MacOSX")
	-( cd "$(OO_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(OO_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
else
	-cat "$<" | unix2dos | ( cd "$(OO_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" )
	cat "$<" | unix2dos | ( cd "$(OO_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" )
endif
	touch "$@"

build.oo_configure: build.oo_patches
ifeq ("$(OS_TYPE)","MacOSX")
	cd "$(OO_BUILD_HOME)" ; setenv PATH "/bin:/sbin:/usr/bin:/usr/sbin:/opt/local/bin" ; unsetenv DYLD_LIBRARY_PATH ; autoconf ; ./configure --without-stlport --with-dmake-url="file://$(PWD)/$(APACHE_PATCHES_HOME)/$(DMAKE_SOURCE_FILENAME)" --with-epm-url="file://$(PWD)/$(MSWEET_PATCHES_HOME)/$(EPM_SOURCE_FILENAME)" --with-jdk-home="$(JDK_HOME)" --with-ant-home="$(PWD)/$(BUILD_HOME)/$(ANT_PACKAGE)" --without-junit --disable-cairo --disable-cups --disable-gtk --disable-odk --with-gnu-cp=/opt/local/bin/gcp --with-system-curl --with-system-odbc --with-lang="$(OO_LANGUAGES)" --disable-fontconfig --without-fonts --without-ppds --without-afms --enable-crashdump=no --enable-category-b --enable-bundled-dictionaries --enable-pdfimport --with-system-poppler --enable-report-builder
else
ifndef JDK_HOME
	@echo "JDK_HOME must be defined in custom.mk" ; exit 1
endif
	@sh -c -e 'if [ ! -r "$(JDK_HOME)/lib/tools.jar" ] ; then echo "You must set JDK_HOME to the path of a valid Java Development Kit" ; exit 1 ; fi'
	( cd "$(BUILD_HOME)/$(OO_PACKAGE)" ; unsetenv LD_LIBRARY_PATH ; ./configure TMP=$(TMP) WGET=/usr/bin/wget --with-distro=Win32 --disable-atl --disable-activex --disable-layout --with-java --with-jdk-home="$(JDK_HOME)" --with-java-target-version=1.6 --with-epm=internal --disable-cairo --disable-cups --disable-gtk --disable-odk --without-nas --with-lang="$(OO_LANGUAGES)" --disable-access --disable-headless --disable-pasf --disable-fontconfig --without-fonts --without-ppds --without-afms --enable-binfilter --enable-extensions --enable-minimizer --enable-presenter-console --enable-pdfimport --enable-ogltrans --enable-report-builder --with-sun-templates )
endif
	touch "$@"

build.oo_all: build.oo_configure
ifeq ("$(OS_TYPE)","MacOSX")
	cd "$(OO_BUILD_HOME)" ; ./bootstrap
	source "$(OO_ENV_AQUA)" ; cd "$(OO_BUILD_HOME)/instsetoo_native" ; perl "$$SOLARENV/bin/build.pl" --all
else
# Copy Visual Studio 9.0 dbghelp.ddl
	sh -e -c 'if [ ! -f "$(OO_BUILD_HOME)/external/dbghelp/dbghelp.dll" ] ; then cp "/cygdrive/c/Program Files/Microsoft Visual Studio 9.0/Common7/IDE/dbghelp.dll" "$(OO_BUILD_HOME)/external/dbghelp/dbghelp.dll" ; fi'
# Prepend Visual Studio 9.0 tools to path
	cd "$(BUILD_HOME)/$(OO_PACKAGE)" ; setenv PATH "/cygdrive/c/Program Files/Microsoft Visual Studio 9.0/VC/bin:$$PATH" ; "$(MAKE)" build
endif
	touch "$@"

build.neo_configure: build.oo_all neo_configure.mk
	$(MAKE) $(MFLAGS) build.neo_configure_phony
	touch "$@"

build.neo_patches:
	build.remotecontrol_patches \
	$(PRODUCT_COMPONENT_MODULES:%=build.neo_%_component) \
	$(PRODUCT_COMPONENT_PATCH_MODULES:%=build.neo_%_component) \
	build.neo_avmedia_patch \
	build.neo_basic_patch \
	build.neo_canvas_patch \
	build.neo_connectivity_patch \
	build.neo_cppcanvas_patch \
	build.neo_cppuhelper_patch \
	build.neo_dbaccess_patch \
	build.neo_desktop_patch \
	build.neo_drawinglayer_patch \
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
	build.neo_sd_patch \
	build.neo_sdext_patch \
	build.neo_extensions_patch build.neo_shell_patch build.neo_sfx2_patch \
	build.neo_sot_patch \
	build.neo_store_patch \
	build.neo_svtools_patch build.neo_sc_patch \
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
	build.neo_libwpd_patch build.neo_writerperfect_patch \
	build.neo_xmloff_patch \
	build.neo_writerfilter_patch
	touch "$@"

build.neo_%_patch: % build.neo_configure
	rm -Rf "$(PWD)/$</$(UOUTPUTDIR)"
	cd "$<" ; sh -e -c '( cd "$(PWD)/$(OO_BUILD_HOME)/$<" ; find . -type d | sed "s/ /\\ /g" | grep -v /CVS$$ | grep -v /$(UOUTPUTDIR) ) | while read i ; do mkdir -p "$$i" ; done'
ifeq ("$(OS_TYPE)","MacOSX")
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

build.remotecontrol_checkout: build.neo_configure
	rm -Rf "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)"
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; mkdir "$(REMOTECONTROL_PACKAGE)"
	cd "$(BUILD_HOME)" ; tar zxvf "$(PWD)/$(REMOTECONTROL_PATCHES_HOME)/$(REMOTECONTROL_SOURCE_FILENAME)"
	touch "$@"

build.remotecontrol_src_untar: $(REMOTECONTROL_PATCHES_HOME)/additional_source build.remotecontrol_checkout
	cd "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)" ; ( cd "$(PWD)/$<" ; tar cf - *.h *.m *.png *.lproj *.plist *.xcodeproj ) | tar xvf -
	touch "$@"

ifeq ("$(OS_TYPE)","MacOSX")
build.remotecontrol_patches: $(REMOTECONTROL_PATCHES_HOME)/remotecontrol.patch build.remotecontrol_src_untar
	-( cd "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	cd "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)" ; xcodebuild -project RemoteControlFramework.xcodeproj -target RemoteControl -configuration Release clean
	cd "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)" ; xcodebuild -project RemoteControlFramework.xcodeproj -target RemoteControl -configuration Release
	touch "$@"
else
build.remotecontrol_patches:
	touch "$@"
endif
	
# End of converted make rules

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
	sh -e -c 'if [ -d "/Volumes/OpenOffice.org $(OO_PRODUCT_VERSION_FAMILY)" ] ; then hdiutil eject -force "/Volumes/OpenOffice.org $(OO_PRODUCT_VERSION_FAMILY)" ; fi'
	hdiutil attach -nobrowse "$(OO_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice/dmg/install/en-US/OOo_$(OO_PRODUCT_VERSION)_"*"$(ULONGNAME)_install.dmg"
	mkdir -p "$(INSTALL_HOME)/package/Contents"
	cd "$(INSTALL_HOME)/package" ; ( ( cd "/Volumes/OpenOffice.org $(OO_PRODUCT_VERSION_FAMILY)/OpenOffice.org.app" ; gnutar cvf - . ) | ( cd "$(PWD)/$(INSTALL_HOME)/package" ; gnutar xvf - --exclude="._*" ) )
	hdiutil eject -force "/Volumes/OpenOffice.org $(OO_PRODUCT_VERSION_FAMILY)"
# Remove OOo system plugins but fix bug 3381 to save standard dictionaries
	rm -Rf "$(INSTALL_HOME)/package/Contents/Frameworks"
	rm -Rf "$(INSTALL_HOME)/package/Contents/Library"
	rm -Rf "$(INSTALL_HOME)/package/Contents/etc"
	rm -Rf "$(INSTALL_HOME)/package/Contents/share/extension"
	rm -Rf "$(INSTALL_HOME)/package/Contents/share/uno_packages"
	mkdir -p "$(INSTALL_HOME)/package/Contents/etc"
	mkdir -p "$(INSTALL_HOME)/package/Contents/share/extension"
	mkdir -p "$(INSTALL_HOME)/package/Contents/share/uno_packages"
# Regroup the OOo language packs
	cd "$(OO_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/install" ; find . -type d -maxdepth 1 -exec basename {} \; | grep -v '^\.$$' | grep -v '^follow_me$$' | grep -v '^log$$' > "$(PWD)/$(INSTALL_HOME)/language_names"
# Include certain languages in the main installer
	sh -e -c 'for i in $(PRODUCT_BUNDLED_LANG_PACKS) ; do if [ -d "/Volumes/OpenOffice.org $(OO_PRODUCT_VERSION_FAMILY)" ] ; then hdiutil eject -force "/Volumes/OpenOffice.org Languagepack" ; fi ; hdiutil attach -nobrowse "$(PWD)/$(OO_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/install/$${i}/OpenOffice.org-langpack-$(OO_PRODUCT_VERSION)_$${i}.dmg" ; bunzip2 -dc "/Volumes/OpenOffice.org Languagepack/OpenOffice.org Languagepack.app/Contents/tarball.tar.bz2" | ( cd "$(PWD)/$(INSTALL_HOME)/package" ; gnutar xvf - --exclude="._*" ) ; hdiutil eject -force "/Volumes/OpenOffice.org Languagepack" ; helpflag=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | awk -F, "{ print \\$$2 }"` ; if [ "$${helpflag}" != "1" ] ; then rm -Rf "$(PWD)/$(INSTALL_HOME)/package/Contents/help/$${i}" ; ( cd "$(PWD)/$(INSTALL_HOME)/package/Contents/help" ; ln -s "en" "$${i}" ) ; fi ; done'
ifndef LANGPACKS
# Bypass the language pack installers
else
# Create the language pack installers
	sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names" | sed $(foreach BUNDLED_LANG_PACK,$(PRODUCT_BUNDLED_LANG_PACKS),-e "/^$(BUNDLED_LANG_PACK)\\$$/d")` ; do langname=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | sed "s/#.*$$//" | awk -F, "{ print \\$$3 }"` ; langdirname=`echo "$${langname}" | sed "s# #_#g"` ; if [ -z "$${langname}" -o -z "$${langdirname}" ] ; then echo "Skipping $${i} language..." ; continue ; fi ; mkdir -p "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents" ; if [ -d "/Volumes/OpenOffice.org $(OO_PRODUCT_VERSION_FAMILY)" ] ; then hdiutil eject -force "/Volumes/OpenOffice.org Languagepack" ; fi ; hdiutil attach -nobrowse "$(PWD)/$(OO_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/install/$${i}/OpenOffice.org-langpack-$(OO_PRODUCT_VERSION)_$${i}.dmg" ; bunzip2 -dc "/Volumes/OpenOffice.org Languagepack/OpenOffice.org Languagepack.app/Contents/tarball.tar.bz2" | ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}" ; gnutar xvf - --exclude="._*" ) ; hdiutil eject -force "/Volumes/OpenOffice.org Languagepack" ; rm -f "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/MacOS/resource/ooo"*.res "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/basis-link/program/resource/dba"*.res ; helpflag=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | awk -F, "{ print \\$$2 }"` ; if [ "$${helpflag}" != "1" ] ; then rm -Rf "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help/$${i}" ; ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help" ; ln -s "en" "$${i}" ) ; fi ; "$(MAKE)" $(MFLAGS) "PRODUCT_LANG_PACK_LOCALE=$${i}" "PRODUCT_LANG_PACK_VERSION=$(PRODUCT_LANG_PACK_VERSION) $${langname}" "PRODUCT_DIR_LANG_PACK_VERSION=$(PRODUCT_DIR_LANG_PACK_VERSION)_$${langdirname}" "build.package_$${langdirname}" ; done'
endif
	chmod -Rf u+w,a+r "$(INSTALL_HOME)/package"
	cd "$(INSTALL_HOME)/package/Contents" ; mv -f "MacOS/resource" "etc" ; ln -sf "../etc/resource" "MacOS/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/avmedia/$(UOUTPUTDIR)/lib/libavmedia$(DLLSUFFIX).dylib" "$(PWD)/avmedia/$(UOUTPUTDIR)/lib/libavmediajava.dylib" "$(PWD)/basic/$(UOUTPUTDIR)/lib/libsb$(DLLSUFFIX).dylib" "$(PWD)/canvas/$(UOUTPUTDIR)/lib/vclcanvas.uno.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libcalc$(DLLSUFFIX).dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libdbase$(DLLSUFFIX).dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libdbtools$(DLLSUFFIX).dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libmacab1.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libmacabdrv1.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libodbc$(DLLSUFFIX).dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libodbcbase$(DLLSUFFIX).dylib" "$(PWD)/cppcanvas/$(UOUTPUTDIR)/lib/libcppcanvas$(DLLSUFFIX).dylib" "$(PWD)/dbaccess/$(UOUTPUTDIR)/lib/libdba$(DLLSUFFIX).dylib" "$(PWD)/dbaccess/$(UOUTPUTDIR)/lib/libdbu$(DLLSUFFIX).dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/deployment$(DLLSUFFIX).uno.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/deploymentgui$(DLLSUFFIX).uno.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libdeploymentmisc$(DLLSUFFIX).dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libsofficeapp.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libspl$(DLLSUFFIX).dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libunopkgapp.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/migrationoo2.uno.dylib" "$(PWD)/drawinglayer/$(UOUTPUTDIR)/lib/libdrawinglayer$(DLLSUFFIX).dylib" "$(PWD)/dtrans/$(UOUTPUTDIR)/lib/libdtransjava$(DLLSUFFIX).dylib" "$(PWD)/extensions/$(UOUTPUTDIR)/lib/libscn$(DLLSUFFIX).dylib" "$(PWD)/filter/$(UOUTPUTDIR)/lib/libpdffilter$(DLLSUFFIX).dylib" "$(PWD)/formula/$(UOUTPUTDIR)/lib/libfor$(DLLSUFFIX).dylib" "$(PWD)/fpicker/$(UOUTPUTDIR)/lib/fpicker.uno.dylib" "$(PWD)/fpicker/$(UOUTPUTDIR)/lib/fps_java.uno.dylib" "$(PWD)/framework/$(UOUTPUTDIR)/lib/libfwe$(DLLSUFFIX).dylib" "$(PWD)/framework/$(UOUTPUTDIR)/lib/libfwk$(DLLSUFFIX).dylib" "$(PWD)/goodies/$(UOUTPUTDIR)/lib/libgo$(DLLSUFFIX).dylib" "$(PWD)/goodies/$(UOUTPUTDIR)/lib/libipt$(DLLSUFFIX).dylib" "$(PWD)/hwpfilter/$(UOUTPUTDIR)/lib/libhwp.dylib" "$(PWD)/i18npool/$(UOUTPUTDIR)/lib/i18npool.uno.dylib" "$(PWD)/lingucomponent/$(UOUTPUTDIR)/lib/libspell$(DLLSUFFIX).dylib" "$(PWD)/linguistic/$(UOUTPUTDIR)/lib/liblng$(DLLSUFFIX).dylib" "$(PWD)/moz/$(UOUTPUTDIR)/lib/libfreebl3.dylib" "$(PWD)/moz/$(UOUTPUTDIR)/lib/libsoftokn3.dylib" "$(PWD)/oox/$(UOUTPUTDIR)/lib/liboox$(DLLSUFFIX).dylib" "$(PWD)/package/$(UOUTPUTDIR)/lib/libpackage2.dylib" "$(PWD)/package/$(UOUTPUTDIR)/lib/libxstor.dylib" "$(PWD)/reportdesign/$(UOUTPUTDIR)/lib/librpt$(DLLSUFFIX).dylib" "$(PWD)/sax/$(UOUTPUTDIR)/lib/fastsax.uno.dylib" "$(PWD)/sc/$(UOUTPUTDIR)/lib/libsc$(DLLSUFFIX).dylib" "$(PWD)/sc/$(UOUTPUTDIR)/lib/libscui$(DLLSUFFIX).dylib" "$(PWD)/sd/$(UOUTPUTDIR)/lib/libpptx$(DLLSUFFIX).dylib" "$(PWD)/sd/$(UOUTPUTDIR)/lib/libsd$(DLLSUFFIX).dylib" "$(PWD)/sd/$(UOUTPUTDIR)/lib/libsdui$(DLLSUFFIX).dylib" "$(PWD)/sfx2/$(UOUTPUTDIR)/lib/libsfx$(DLLSUFFIX).dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/cmdmail.uno.dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/localebe1.uno.dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/macbe1.uno.dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/syssh.uno.dylib" "$(PWD)/sot/$(UOUTPUTDIR)/lib/libsot$(DLLSUFFIX).dylib" "$(PWD)/svtools/$(UOUTPUTDIR)/lib/libsvl$(DLLSUFFIX).dylib" "$(PWD)/svtools/$(UOUTPUTDIR)/lib/libsvt$(DLLSUFFIX).dylib" "$(PWD)/svx/$(UOUTPUTDIR)/lib/libcui$(DLLSUFFIX).dylib" "$(PWD)/svx/$(UOUTPUTDIR)/lib/libsvx$(DLLSUFFIX).dylib" "$(PWD)/svx/$(UOUTPUTDIR)/lib/libsvxcore$(DLLSUFFIX).dylib" "$(PWD)/svx/$(UOUTPUTDIR)/lib/libsvxmsfilter$(DLLSUFFIX).dylib" "$(PWD)/sw/$(UOUTPUTDIR)/lib/libdocx$(DLLSUFFIX).dylib" "$(PWD)/sw/$(UOUTPUTDIR)/lib/libmsword$(DLLSUFFIX).dylib" "$(PWD)/sw/$(UOUTPUTDIR)/lib/libsw$(DLLSUFFIX).dylib" "$(PWD)/sw/$(UOUTPUTDIR)/lib/libswui$(DLLSUFFIX).dylib" "$(PWD)/tools/$(UOUTPUTDIR)/lib/libtl$(DLLSUFFIX).dylib" "$(PWD)/ucb/$(UOUTPUTDIR)/lib/libucpdav1.dylib" "$(PWD)/ucb/$(UOUTPUTDIR)/lib/libucpfile1.dylib" "$(PWD)/ucbhelper/$(UOUTPUTDIR)/lib/libucbhelper4gcc3.dylib" "$(PWD)/unotools/$(UOUTPUTDIR)/lib/libutl$(DLLSUFFIX).dylib" "$(PWD)/unoxml/$(UOUTPUTDIR)/lib/libunordf$(DLLSUFFIX).dylib" "$(PWD)/unoxml/$(UOUTPUTDIR)/lib/libunoxml$(DLLSUFFIX).dylib" "$(PWD)/uui/$(UOUTPUTDIR)/lib/libuui$(DLLSUFFIX).dylib" "$(PWD)/vcl/$(UOUTPUTDIR)/lib/libvcl$(DLLSUFFIX).dylib" "$(PWD)/vos/$(UOUTPUTDIR)/lib/libvos3gcc3.dylib" "$(PWD)/writerfilter/$(UOUTPUTDIR)/lib/libwriterfilter$(DLLSUFFIX).dylib" "$(PWD)/writerperfect/$(UOUTPUTDIR)/lib/libmsworks$(DLLSUFFIX).dylib" "$(PWD)/writerperfect/$(UOUTPUTDIR)/lib/libwpft$(DLLSUFFIX).dylib" "$(PWD)/writerperfect/$(UOUTPUTDIR)/lib/libwpgimport$(DLLSUFFIX).dylib" "$(PWD)/xmloff/$(UOUTPUTDIR)/lib/libxo$(DLLSUFFIX).dylib" "basis-link/program"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/extensions/$(UOUTPUTDIR)/lib/libupdchk$(DLLSUFFIX).dylib" "$(PWD)/extensions/$(UOUTPUTDIR)/lib/updchk.uno.dylib" "$(PWD)/pyuno/$(UOUTPUTDIR)/lib/libpyuno.dylib" "basis-link/program"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/cppuhelper/$(UOUTPUTDIR)/lib/libuno_cppuhelpergcc3.dylib.3" "$(PWD)/jvmfwk/$(UOUTPUTDIR)/lib/libjvmfwk.dylib.3" "$(PWD)/sal/$(UOUTPUTDIR)/lib/libuno_sal.dylib.3" "$(PWD)/salhelper/$(UOUTPUTDIR)/lib/libuno_salhelpergcc3.dylib.3" "$(PWD)/store/$(UOUTPUTDIR)/lib/libstore.dylib.3" "basis-link/ure-link/lib"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/jvmfwk/$(UOUTPUTDIR)/lib/sunjavaplugin.dylib" "basis-link/ure-link/lib"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/vcl/$(UOUTPUTDIR)/bin/salapp"*.res "etc/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/cpputools/$(UOUTPUTDIR)/bin/uno" "basis-link/ure-link/bin/uno.bin" ; chmod a+x "basis-link/ure-link/bin/uno.bin"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/vcl/$(UOUTPUTDIR)/bin/checknativefont" "basis-link/program/checknativefont" ; chmod a+x "basis-link/program/checknativefont"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice3" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/extensions/$(UOUTPUTDIR)/bin/updchkruninstallers" "MacOS/updchkruninstallers.bin" ; chmod a+x "MacOS/updchkruninstallers.bin"
else ifdef PRODUCT_BUILD2
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice2" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
else
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
endif
# Mac App Store will reject apps with shell scripts
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in sbase scalc sdraw simpress smath soffice swriter unopkg unopkg.bin ; do rm -f "MacOS/$$i" ; ln -sf "soffice.bin" "MacOS/$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "basis-link/program/gengal" ; ln -sf "gengal.bin" "basis-link/program/gengal"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in regcomp uno ; do rm -f "basis-link/ure-link/bin/$$i" ; ln -sf "$$i.bin" "basis-link/ure-link/bin/$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; cp -f "$(PWD)/sfx2/$(UOUTPUTDIR)/bin/objserv_cocoa"*.res "etc/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp -f "$(PWD)/sfx2/$(UOUTPUTDIR)/bin/shutdowniconjava"*.res "etc/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp -f "$(PWD)/sw/$(UOUTPUTDIR)/bin/swmacdictlookup"*.res "etc/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "etc/resource/ooo"*.res ; cp -f "$(PWD)/svx/$(UOUTPUTDIR)/bin/ooo"*.res "etc/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "basis-link/program/resource/dba"*.res ; cp -f "$(PWD)/dbaccess/$(UOUTPUTDIR)/bin/dba"*.res "basis-link/program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "basis-link/program/resource/deploymentgui"*.res ; cp -f "$(PWD)/desktop/$(UOUTPUTDIR)/bin/deploymentgui"*.res "basis-link/program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in editpic.pl poll.pl savepic.pl show.pl ; do rm -f "basis-link/share/config/webcast/$$i" ; cp -f "$(PWD)/sd/res/webview/$$i" "basis-link/share/config/webcast/$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "basis-link/share/config/webcast/webcast.pl" ; cp -f "$(PWD)/sd/res/webview/webview.pl" "basis-link/share/config/webcast/webcast.pl"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/shell/$(UOUTPUTDIR)/bin/senddoc" "basis-link/program/senddoc" ; chmod a+x "basis-link/program/senddoc"
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
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#^ProgressPosition=.*$$#ProgressPosition=14,260#g' "MacOS/sofficerc" > "etc/sofficerc" ; rm -f "MacOS/sofficerc" ; ln -sf "../etc/sofficerc" "MacOS/sofficerc"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in bootstraprc fundamentalrc setuprc versionrc ; do mv -f "MacOS/$$i" "etc/$$i" ; ln -sf "../etc/$$i" "MacOS/$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find "basis-link/share/config/soffice.cfg/modules" -name "menubar.xml"` ; do sed "s#<menu:menuitem.*\.uno:TwainSelect.*/>#<\!--&-->#g" "$${i}" > "../../out" ; mv -f "../../out" "$${i}" ; done'
	rm -Rf "$(INSTALL_HOME)/package/Contents/Resources"
	mkdir -p "$(INSTALL_HOME)/package/Contents/Resources/cursors"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/vcl/java/com/sun/star/vcl/images/"*.gif "Resources/cursors"
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; ( ( cd "$(PWD)/vcl/java/source/res" ; gnutar cvf - --exclude CVS MainMenu.nib ) | gnutar xvf - )
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/neo2toolbarv101.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0.1/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" .
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
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2012_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/NeoOfficeAquaElements.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "MacOS/about.bmp" ; cp "tmp/NeoOffice Aqua Elements 3/Contents/MacOS/about.bmp" "etc/about.bmp" ; ln -sf "../etc/about.bmp" "MacOS/about.bmp"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "MacOS/intro.bmp" ; cp "$(PWD)/etc/package/intro_classic.bmp" "etc/intro.bmp" ; ln -sf "../etc/intro.bmp" "MacOS/intro.bmp"
else ifdef PRODUCT_BUILD2
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "MacOS/intro.bmp" ; cp "$(PWD)/etc/package/intro_free.bmp" "etc/intro.bmp" ; ln -sf "../etc/intro.bmp" "MacOS/intro.bmp"
else
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "MacOS/intro.bmp" ; cp "tmp/NeoOffice Aqua Elements 3/Contents/MacOS/intro.bmp" "etc/intro.bmp" ; ln -sf "../etc/intro.bmp" "MacOS/intro.bmp"
endif
endif
	cd "$(INSTALL_HOME)/package/Contents" ; cp "tmp/NeoOffice Aqua Elements 3/Contents/Resources/"*.icns "Resources"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/package/ship.icns" "Resources"
endif
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"` svtools svx
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"` svtools svx
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libAppleRemote$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libMacOSXSpell$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/classes/avmedia.jar'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libavmediaQuickTime$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libavmediajava.dylib'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libdtransaqua$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libdtransjava$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/fps_aqua.uno.dylib'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/fps_java.uno.dylib'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/updchk.uno.dylib'
# Do not ship the Lotus Word Pro filter as it is very unstable on Mac OS X
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/liblwpft$(DLLSUFFIX).dylib'
# Reregister hwpfilter
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libhwp.dylib'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libhwp.dylib'
# Reregister spellchecker to enable native grammarchecker
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libspell$(DLLSUFFIX).dylib'
ifndef PRODUCT_BUILD3
# Revoke HSQLDB driver
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libhsqldb.dylib'
# Revoke JDBC driver
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libjdbcmxi.dylib'
# Revoke Java services
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; sh -e -c 'for i in LuceneHelpWrapper.jar ScriptFramework.jar ScriptProviderForBeanShell.jar ScriptProviderForJava.jar ScriptProviderForJavaScript.jar XMergeBridge.jar XSLTFilter.jar XSLTValidate.jar agenda.jar fax.jar form.jar letter.jar query.jar report.jar table.jar web.jar ; do "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c "vnd.sun.star.expand:\$$OOO_BASE_DIR/program/classes/$$i" ; done'
# Revoke Python services
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.openoffice.pymodule:mailmerge'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.openoffice.pymodule:pythonscript'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/pythonloader.uno.dylib'
endif
# Add Mac OS X localized resources
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; sh -e -c 'for i in `echo "$(PRODUCT_BUNDLED_LANG_PACKS)" | sed "s#-#_#g"` ; do mkdir -p "$${i}.lproj" ; mkdir -p `echo "$${i}" | sed "s#_.*\\$$##"`".lproj" ; done'
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; ( ( cd "$(PWD)/etc/package/l10n" ; gnutar cvf - --exclude CVS --exclude "*.html" . ) | gnutar xvf - )
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; sh -e -c 'for i in `cd "$(PWD)/etc/package/l10n" ; find . -name "*.html"` ; do sed "s#\$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g" "$(PWD)/etc/package/l10n/$${i}" | sed "s#\$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g" | sed "s#\$$(PRODUCT_DOWNLOAD_URL)#$(PRODUCT_DOWNLOAD_URL)#g" | sed "s#\$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g" | sed "s#\$$(PRODUCT_MAX_OSVERSION)#$(PRODUCT_MAX_OSVERSION)#g" > "$${i}" ; done'
endif
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "MacOS/unoinfo" "basis-link/program/open-url" "basis-link/program/senddoc" LICENSE* README* basis-link/licenses basis-link/readmes "basis-link/ure-link/bin/startup.sh" share/readme
# Fix bug 3273 by not installing any OOo or ooo-build fonts
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "basis-link/program/libAppleRemote$(DLLSUFFIX).dylib" "basis-link/program/libMacOSXSpell$(DLLSUFFIX).dylib" "basis-link/program/libavmediaQuickTime$(DLLSUFFIX).dylib" "basis-link/program/libdtransaqua$(DLLSUFFIX).dylib" "basis-link/program/fps_aqua.uno.dylib" "basis-link/program/liblwpft$(DLLSUFFIX).dylib" "basis-link/share/fonts/truetype" "basis-link/share/psprint"
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/jvmfwk/$(UOUTPUTDIR)/bin/javavendors.xml" "basis-link/ure-link/share/misc/javavendors.xml"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/extensions/$(UOUTPUTDIR)/misc/registry/spool/org/openoffice/Office/Addons/Addons-onlineupdate.xcu" "basis-link/share/registry/data/org/openoffice/Office/Addons.xcu"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/extensions/$(UOUTPUTDIR)/misc/registry/spool/org/openoffice/Office/Jobs/Jobs-onlineupdate.xcu" "basis-link/share/registry/data/org/openoffice/Office/Jobs.xcu"
else
# Remove all update check files since the Mac App Store has its own update
# check. Do not remove updatefeed.uno.dylib as it is needed by the pdfimport
# extension.
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "basis-link/program/libupdchk$(DLLSUFFIX).dylib" "basis-link/share/registry/data/org/openoffice/Office/Addons.xcu" "basis-link/share/registry/data/org/openoffice/Office/Jobs.xcu"
# Remove Java files
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "basis-link/program/classes" "basis-link/program/libhsqldb.dylib" "basis-link/program/libjdbc$(DLLSUFFIX).dylib" "basis-link/share/Scripts/java" "basis-link/share/registry/modules/org/openoffice/TypeDetection/Filter/fcfg_palm_filters.xcu" "basis-link/share/registry/modules/org/openoffice/TypeDetection/Filter/fcfg_pocketexcel_filters.xcu" "basis-link/share/registry/modules/org/openoffice/TypeDetection/Filter/fcfg_pocketword_filters.xcu" "basis-link/share/registry/modules/org/openoffice/TypeDetection/Types/fcfg_palm_types.xcu" "basis-link/share/registry/modules/org/openoffice/TypeDetection/Types/fcfg_pocketexcel_types.xcu" "basis-link/share/registry/modules/org/openoffice/TypeDetection/Types/fcfg_pocketword_types.xcu" "basis-link/ure-link/share/java" "basis-link/ure-link/lib/JREProperties.class" "basis-link/ure-link/lib/javaloader.uno.dylib" "basis-link/ure-link/lib/javavm.uno.dylib" "basis-link/ure-link/lib/libjava_uno.dylib" "basis-link/ure-link/lib/libjuh.dylib" "basis-link/ure-link/lib/libjuhx.dylib" "basis-link/ure-link/lib/sunjavaplugin.dylib" "basis-link/ure-link/share/java" "basis-link/ure-link/share/misc/javavendors.xml"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . ! -type d -name "*.jnilib"` ; do rm -f $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents/basis-link/help" ; sh -e -c 'for i in `find . -type d -name "*.idxl"` ; do rm -Rf $${i} ; done'
# Remove Python files
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "basis-link/program/libpyuno.dylib" "basis-link/program/pythonloader.uno.dylib" "basis-link/program/mailmerge.py" "basis-link/program/officehelper.py" "basis-link/program/pythonloader.unorc" "basis-link/program/pythonloader.py" "basis-link/program/pythonscript.py" "basis-link/program/uno.py" "basis-link/program/pyuno.so" "basis-link/program/unohelper.py" "basis-link/share/Scripts/python" "basis-link/share/registry/modules/org/openoffice/Office/Scripting/Scripting-python.xcu"
endif
	cd "$(INSTALL_HOME)/package/Contents/basis-link" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share -type f -not -path "*/CVS/*" | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents/basis-link" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share -type f -not -path "*/CVS/*"` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' | sed 's#BuildMachine=.*$$#BuildMachine=$(BUILD_MACHINE)#g' > "../../out" ; mv -f "../../out" "etc/bootstraprc"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(TARGET_MACHINE)#$(TARGET_MACHINE)#g' "$(PWD)/etc/program/fundamentalbasisrc" > "basis-link/program/fundamentalbasisrc"
else
	cd "$(INSTALL_HOME)/package/Contents/basis-link" ; sh -e -c 'for i in `cd "$(PWD)/etc/sandbox" ; find share -type f -not -path "*/CVS/*" | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents/basis-link" ; sh -e -c 'for i in `cd "$(PWD)/etc/sandbox" ; find share -type f -not -path "*/CVS/*"` ; do cp "$(PWD)/etc/sandbox/$${i}" "$${i}" ; done'
# Remove report toolbar from Base since reports require Java support
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find "basis-link/share/registry" -name BaseWindowState.xcu` ; do sed "s#\"private:resource\/toolbar\/reportobjectbar\"#\"ignore\"#g" "$${i}" > "../../out" ; mv -f "../../out" "$${i}" ; done'
	rm "$(INSTALL_HOME)/package/Contents/basis-link/share/config/soffice.cfg/modules/dbapp/toolbar/reportobjectbar.xml"
	cd "$(INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/$(PRODUCT_DIR_NAME)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' | sed 's#BuildMachine=.*$$#BuildMachine=$(BUILD_MACHINE)#g' > "../../out" ; mv -f "../../out" "etc/bootstraprc"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/program/fundamentalrc" "etc/fundamentalrc"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/program/versionrc" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_UPDATE_CHECK_URL)#$(PRODUCT_UPDATE_CHECK_URL)#g' | sed 's# #%20#g' | sed 's#^buildid=.*$$#buildid=$(PRODUCT_PATCH_VERSION)#' > "etc/versionrc"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in "share/registry/data/org/openoffice/Office/Compatibility.xcu" "share/registry/modules/org/openoffice/Office/Common/Common-brand.xcu" "share/registry/modules/org/openoffice/Office/UI/UI-brand.xcu" "share/registry/modules/org/openoffice/Setup/Setup-brand.xcu" ; do sed "s#oor:name=\"$(OO_PRODUCT_NAME)\"#oor:name=\"$(PRODUCT_NAME)\"#g" "$${i}" | sed "s#>$(OO_PRODUCT_NAME)<#>$(PRODUCT_NAME)<#g" | sed "s#>$(OO_PRODUCT_VERSION_FAMILY)<#>$(PRODUCT_VERSION)<#g" | sed "s#>$(OO_PRODUCT_VERSION)<#>$(PRODUCT_VERSION)<#g" | sed "s#>$(OO_REGISTRATION_URL)<#>$(PRODUCT_REGISTRATION_URL)<#g" > "../../out" ; mv -f "../../out" "$${i}" ; done'
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(OO_PRODUCT_NAME)#$(OO_PRODUCT_NAME)#g' "$(PWD)/etc/help/main_transform.xsl" | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL)#$(PRODUCT_DOCUMENTATION_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL_TEXT)#$(PRODUCT_DOCUMENTATION_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#g' > "basis-link/help/main_transform.xsl"
else
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(OO_PRODUCT_NAME)#$(OO_PRODUCT_NAME)#g' "$(PWD)/etc/sandbox/help/main_transform.xsl" | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL)#$(PRODUCT_DOCUMENTATION_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL_TEXT)#$(PRODUCT_DOCUMENTATION_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#g' > "basis-link/help/main_transform.xsl"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.bin"` ; do strip -S -x "$$i" ; done'
# Mac OS 10.5.x and higher cannot strip the Mozilla libraries to exclude them
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*" | grep -v "components"` ; do strip -S -x "$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.so"` ; do strip -S -x "$$i" ; done'
# Integrate the RemoteControl framework
	mkdir -p "$(INSTALL_HOME)/package/Contents/Frameworks"
	cd "$(INSTALL_HOME)/package" ; ( ( cd "$(PWD)/$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)/build/Release" ; gnutar cvf - --exclude Headers RemoteControl.framework ) | ( cd "$(PWD)/$(INSTALL_HOME)/package/Contents/Frameworks" ; gnutar xvf - ; strip -S -x RemoteControl.framework/Versions/A/RemoteControl ) )
# Install OOo .oxt files. Note that we exclude the wiki-publisher.oxt file as
# has been found to have buggy network connectivity and we exclude
# sun-report-builder.oxt because it requires Java. Fix crashing bug by using
# our patched build of pdfimport.oxt. Fix bug 3501 by using our patched build
# of sun-presentation-minimizer.oxt.
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -c -e 'JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1 ; export JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY ; unset CLASSPATH ; unset DYLD_LIBRARY_PATH ; for i in `find "$(PWD)/$(OO_BUILD_HOME)/solver/$${UPD}/$(UOUTPUTDIR)/bin" -type f -name "*.oxt" | grep -v "sun-report-builder.oxt" | grep -v "wiki-publisher.oxt" | grep -v "pdfimport.oxt" | grep -v "sun-presentation-minimizer.oxt"` "$(PWD)/sdext/$(UOUTPUTDIR)/bin/pdfimport.oxt" "$(PWD)/sdext/$(UOUTPUTDIR)/bin/sun-presentation-minimizer.oxt" ; do rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp" ; echo "yes" | ./unopkg.bin add --shared --verbose "$$i" -env:UserInstallation=file://"$(PWD)/$(INSTALL_HOME)/tmp" ; done ; rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp"'
ifdef PRODUCT_BUILD3
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -c -e 'JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1 ; export JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY ; unset CLASSPATH ; unset DYLD_LIBRARY_PATH ; for i in `find "$(PWD)/$(OO_BUILD_HOME)/solver/$${UPD}/$(UOUTPUTDIR)/bin" -type f -name "sun-report-builder.oxt"` ; do rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp" ; echo "yes" | ./unopkg.bin add --shared --verbose "$$i" -env:UserInstallation=file://"$(PWD)/$(INSTALL_HOME)/tmp" ; done ; rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp"'
endif
# Reinstall dictionary .oxt files removed by patches/openoffice/scp2.patch
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -c -e 'JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1 ; export JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY ; unset CLASSPATH ; unset DYLD_LIBRARY_PATH ; for i in `find -E "$(PWD)/dictionaries/$(UOUTPUTDIR)/bin" -type f -name "*.oxt" -regex ".*\/dict-(en|es|fr)\.oxt"` ; do rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp" ; echo "yes" | ./unopkg.bin add --shared --verbose "$$i" -env:UserInstallation=file://"$(PWD)/$(INSTALL_HOME)/tmp" ; done ; rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp"'
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
	xattr -rc "$(INSTALL_HOME)/package"
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
ifdef PRODUCT_BUILD3
	cd "$(INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" "Contents/MacOS/updchkruninstallers.bin"
	cd "$(INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" .
else
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type f -name "*.bin" \! -name "soffice.bin"` "Contents/basis-link/program/checknativefont" "Contents/basis-link/program/msfontextract" "Contents/basis-link/program/pagein" "Contents/basis-link/program/uri-encode" "Contents/basis-link/ure-link/bin/regmerge" "Contents/basis-link/ure-link/bin/regview" `find "Contents/share/uno_packages/cache/uno_packages" -type f -name "xpdfimport"` ; do codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/etc/package/Entitlements_inherit_only.plist" "$$i" ; done'
	cat "etc/package/Entitlements.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(INSTALL_HOME)/Entitlements.plist"
	cd "$(INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/$(INSTALL_HOME)/Entitlements.plist" .
endif
# Test that all libraries will load
	cd "$(INSTALL_HOME)/package" ; $(CC) -arch x86_64 -o "Contents/MacOS/loaddyliblist" "$(PWD)/etc/package/loaddyliblist.c" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib" ; sh -e -c 'find . -type f -name "*.dylib*" | "Contents/MacOS/loaddyliblist" ; rm -f "Contents/MacOS/loaddyliblist"'
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
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/basis-link/program"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/basis-link/ure-link/lib"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/etc"
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
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/vcl/$(UOUTPUTDIR)/lib/libvcl$(DLLSUFFIX).dylib" "basis-link/program"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/package/Info.plist" | sed 's#$$(PRODUCT_DOMAIN)#$(PRODUCT_DOMAIN)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME)#$(PRODUCT_TRADEMARKED_NAME)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(PRODUCT_FILETYPE)#$(PRODUCT_FILETYPE)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(PRODUCT_BUILD_VERSION)#$(PRODUCT_VERSION_BASE)#g' > "Info.plist"
ifdef PRODUCT_BUILD3
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' | sed 's#BuildMachine=.*$$#BuildMachine=$(BUILD_MACHINE)#g' > "../../out" ; mv -f "../../out" "etc/bootstraprc"
else
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/$(PRODUCT_DIR_NAME)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' | sed 's#BuildMachine=.*$$#BuildMachine=$(BUILD_MACHINE)#g' > "../../out" ; mv -f "../../out" "etc/bootstraprc"
endif
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/program/versionrc" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_UPDATE_CHECK_URL)#$(PRODUCT_UPDATE_CHECK_URL)#g' | sed 's# #%20#g' | sed 's#^buildid=.*$$#buildid=$(PRODUCT_PATCH_VERSION)#' > "etc/versionrc"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.bin"` ; do strip -S -x "$$i" ; done'
# Mac OS 10.5.x and higher cannot strip the Mozilla libraries to exclude them
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*" | grep -v "components"` ; do strip -S -x "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.so"` ; do strip -S -x "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find "." -name ".DS_Store"` ; do rm "$${i}" ; done'
	xattr -rc "$(PATCH_INSTALL_HOME)/package"
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
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type f -name "*.bin" \! -name "soffice.bin"` "Contents/basis-link/program/checknativefont" ; do codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/etc/package/Entitlements_inherit_only.plist" "$$i" ; done'
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
	cd "$</Contents" ; rm -Rf basis-link/licenses basis-link/readmes share/readme
ifndef PRODUCT_BUILD3
# Remove report toolbar from Base since reports require Java support
	cd "$</Contents" ; sh -e -c 'for i in `find "basis-link/share/registry" -name BaseWindowState.xcu` ; do sed "s#\"private:resource\/toolbar\/reportobjectbar\"#\"ignore\"#g" "$${i}" > "../../out" ; mv -f "../../out" "$${i}" ; done'
endif
	rm -Rf "$</Contents/Resources"
	mkdir -p "$</Contents/Resources"
# Add Mac OS X localized resources
	cd "$</Contents/Resources" ; sh -e -c 'for i in `echo "$(PRODUCT_LANG_PACK_LOCALE)" | sed "s#-#_#g"` ; do mkdir -p "$${i}.lproj" ; mkdir -p `echo "$${i}" | sed "s#_.*\\$$##"`".lproj" ; done'
	cd "$<" ; sh -e -c 'for i in `find "." -name ".DS_Store"` ; do rm "$${i}" ; done'
	xattr -rc "$<"
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
