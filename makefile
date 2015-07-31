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
# Use gcc as build will fail with clang
CC=gcc
CXX=g++
EXTRA_PATH=/opt/local/bin
GNUCP=$(EXTRA_PATH)/gcp
LIBIDL_CONFIG=$(EXTRA_PATH)/libIDL-config-2
PKG_CONFIG=$(EXTRA_PATH)/pkg-config
PKG_CONFIG_PATH=$(EXTRA_PATH)/../lib/pkgconfig/
ifeq ("$(shell uname -s)","Darwin")
JDK_HOME=/System/Library/Frameworks/JavaVM.framework/Home
else
# JDK_HOME must be defined in custom.mk
endif
PRODUCT_NAME=My Untested Office Suite
PRODUCT_DIR_NAME=$(subst $(SPACE),,$(PRODUCT_NAME))
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
ifeq ("$(UNAME)","powerpc")
ULONGNAME=PowerPC
CPUNAME=P
UOUTPUTDIR=unxmacxp.pro
DLLSUFFIX=mxp
TARGET_MACHINE=ppc
else
ULONGNAME=Intel
CPUNAME=I
UOUTPUTDIR=unxmacxi.pro
DLLSUFFIX=mxi
TARGET_MACHINE=x86_64
endif
TARGET_FILE_TYPE=Mach-O 64-bit executable $(TARGET_MACHINE)
else
OS_TYPE=Win32
UOUTPUTDIR=wntmsci12.pro
DLLSUFFIX=mxp
endif
BUILD_MACHINE=$(shell echo `id -nu`:`hostname`.`domainname`)

# Build location macros
BUILD_HOME:=build
ifdef PRODUCT_BUILD2
INSTALL_HOME:=install2
PATCH_INSTALL_HOME:=patch_install2
else
INSTALL_HOME:=install
PATCH_INSTALL_HOME:=patch_install
endif
SOURCE_HOME:=source
CD_INSTALL_HOME:=cd_install
MOZILLA_PATCHES_HOME:=patches/mozilla
NEOOFFICE_PATCHES_HOME:=patches/neooffice
OO_PATCHES_HOME:=patches/openoffice
OOO-BUILD_PATCHES_HOME:=patches/ooo-build
OOO-BUILD_PACKAGE=ooo-build-3.1.1.1
OOO-BUILD_BUILD_HOME=$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)/build/ooo310-m19
IMEDIA_PATCHES_HOME:=patches/imedia
REMOTECONTROL_PATCHES_HOME:=patches/remotecontrol
ifeq ("$(OS_TYPE)","MacOSX")
ifeq ("$(UNAME)","powerpc")
OO_ENV_AQUA:=$(OOO-BUILD_BUILD_HOME)/MacOSXPPCEnv.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacOSXPPCEnvJava.Set
else
OO_ENV_AQUA:=$(OOO-BUILD_BUILD_HOME)/MacOSXX86Env.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacOSXX86EnvJava.Set
endif
else
OO_ENV_AQUA:=$(OOO-BUILD_BUILD_HOME)/winenv.set
OO_ENV_JAVA:=$(BUILD_HOME)/winenv.set
endif
COMPILERDIR=$(OOO-BUILD_BUILD_HOME)/solenv/`basename $(UOUTPUTDIR) .pro`/bin
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
PRODUCT_VERSION_BASE=2014
PRODUCT_VERSION=$(PRODUCT_VERSION_BASE).13
PRODUCT_VERSION2=$(PRODUCT_VERSION) $(PRODUCT_VERSION_EXT2)
PRODUCT_VERSION_EXT=
PRODUCT_VERSION_EXT2=Free Edition
PRODUCT_DIR_VERSION=$(subst $(SPACE),_,$(PRODUCT_VERSION))
ifdef PRODUCT_BUILD2
PRODUCT_SHORT_VERSION=$(PRODUCT_VERSION)
else
PRODUCT_SHORT_VERSION=$(subst $(SPACE),,$(subst $(PRODUCT_VERSION_EXT),,$(PRODUCT_VERSION)))
endif
PREVIOUS_PRODUCT_VERSION_BASE=$(PRODUCT_VERSION_BASE)
PREVIOUS_PRODUCT_VERSION=$(PRODUCT_VERSION)
PRODUCT_LANG_PACK_VERSION=Language Pack
PRODUCT_DIR_LANG_PACK_VERSION=$(subst $(SPACE),_,$(PRODUCT_LANG_PACK_VERSION))
PRODUCT_PATCH_VERSION=
PRODUCT_DIR_PATCH_VERSION=$(subst $(SPACE),_,$(PRODUCT_PATCH_VERSION))$(PRODUCT_DIR_PATCH_VERSION_EXTRA)
PRODUCT_MIN_OSVERSION=10.8
PRODUCT_FILETYPE=NO%F
PRODUCT_BASE_URL=http://www.neooffice.org/neojava
PRODUCT_REGISTRATION_URL=http://trinity.neooffice.org/modules.php?name=Your_Account\&amp\;redirect=index
PRODUCT_SUPPORT_URL=http://www.neooffice.org/neojava/macappstore.php
PRODUCT_SUPPORT_URL_TEXT:=$(PRODUCT_NAME) Support
PRODUCT_DOCUMENTATION_URL=http://neowiki.neooffice.org/
PRODUCT_DOCUMENTATION_URL_TEXT=$(PRODUCT_NAME) Wiki
PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL=http://neowiki.neooffice.org/index.php/NeoOffice_Launch_Shortcuts
PRODUCT_DOCUMENTATION_SPELLCHECK_URL=http://neowiki.neooffice.org/index.php/Activating_Dictionaries_and_Configuring_Spellcheck
PRODUCT_UPDATE_CHECK_URL=$(PRODUCT_BASE_URL)/patchcheck.php
PRODUCT_MAC_APP_STORE_URL=$(PRODUCT_BASE_URL)/download.php
PRODUCT_BUNDLED_LANG_PACKS=en-US de fr it he ja ar es ru nl en-GB sv pl nb fi pt-BR da zh-TW cs th zh-CN el hu sk ko
PRODUCT_BUNDLED_LANG_PACKS2=$(PRODUCT_BUNDLED_LANG_PACKS)
ifeq ("$(OS_TYPE)","MacOSX")
PRODUCT_COMPONENT_MODULES+=imagecapture remotecontrol
PRODUCT_COMPONENT_PATCH_MODULES=
PREFLIGHT_REQUIRED_COMMANDS=defaults find id open touch
INSTALLATION_CHECK_REQUIRED_COMMANDS=$(PREFLIGHT_REQUIRED_COMMANDS) awk basename chmod dirname file grep mv pax rm ps pwd sed sort unzip
else
PRODUCT_COMPONENT_MODULES=
PRODUCT_COMPONENT_PATCH_MODULES=
endif

# CVS macros
IMEDIA_PACKAGE=imedia-read-only
IMEDIA_SOURCE_FILENAME=imedia-read-only.tar.gz
REMOTECONTROL_PACKAGE=martinkahr-apple_remote_control-2ba0484
REMOTECONTROL_SOURCE_FILENAME=martinkahr-apple_remote_control.tar.gz
YOURSWAYCREATEDMG_PACKAGE=jaeggir-yoursway-create-dmg-a22ac11
YOURSWAYCREATEDMG_SOURCE_FILENAME=yoursway-create-dmg.zip
NEO_CVSROOT:=:pserver:anoncvs@anoncvs.neooffice.org:/cvs
NEO_PACKAGE:=NeoOffice
NEO_TAG:=NeoOffice-2014_13
NEO_TAG2:=NeoOffice-2014_13_Free_Edition

all: build.all

# Include dependent makefiles
include neo_configure.mk

build.ooo-build_checkout: $(OOO-BUILD_PATCHES_HOME)/$(OOO-BUILD_PACKAGE).tar.gz
	mkdir -p "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)"
	cd "$(BUILD_HOME)" ; tar zxvf "$(PWD)/$<"
	cd "$(BUILD_HOME)" ; chmod -Rf u+rw "$(OOO-BUILD_PACKAGE)"
	touch "$@"

build.sun-source_download: build.ooo-build_checkout
	cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; ( ( cd "$(PWD)/$(OOO-BUILD_PATCHES_HOME)" ; gnutar cvf - --exclude CVS src ) | gnutar xvf - )
	touch "$@"

build.ooo-build_configure: build.ooo-build_checkout build.sun-source_download
# Include OpenOffice.org extenstions and templates. Note that we exclude the
# wiki-publisher.oxt file as it has been found to have buggy network
# connectivity.
ifeq ("$(OS_TYPE)","MacOSX")
	( cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; setenv PATH "$(PWD)/$(COMPILERDIR):/bin:/sbin:/usr/bin:/usr/sbin:$(EXTRA_PATH)" ; unsetenv DYLD_LIBRARY_PATH ; ./configure CC=$(CC) CXX=$(CXX) LIBIDL_CONFIG="$(LIBIDL_CONFIG)" PKG_CONFIG="$(PKG_CONFIG)" PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" TMP=$(TMP) --with-distro=MacOSX --with-java=no --with-jdk-home="$(JDK_HOME)" --with-java-target-version=1.5 --with-epm=internal --disable-cairo --disable-cups --disable-gtk --disable-odk --without-nas --with-mozilla-toolkit=cocoa --with-gnu-cp="$(GNUCP)" --with-system-curl --with-system-odbc-headers --with-lang="$(OO_LANGUAGES)" --disable-access --disable-headless --disable-pasf --disable-fontconfig --without-fonts --without-ppds --without-afms --enable-binfilter --enable-extensions --enable-crashdump=no --enable-minimizer --enable-presenter-console --enable-pdfimport --enable-ogltrans --enable-report-builder --with-sun-templates )
else
ifndef JDK_HOME
	@echo "JDK_HOME must be defined in custom.mk" ; exit 1
endif
	@sh -c -e 'if [ ! -r "$(JDK_HOME)/lib/tools.jar" ] ; then echo "You must set JDK_HOME to the path of a valid Java Development Kit" ; exit 1 ; fi'
	( cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; unsetenv LD_LIBRARY_PATH ; ./configure TMP=$(TMP) WGET=/usr/bin/wget --with-distro=Win32 --disable-atl --disable-activex --disable-layout --with-java --with-jdk-home="$(JDK_HOME)" --with-java-target-version=1.5 --with-epm=internal --disable-cairo --disable-cups --disable-gtk --disable-odk --without-nas --with-lang="$(OO_LANGUAGES)" --disable-access --disable-headless --disable-pasf --disable-fontconfig --without-fonts --without-ppds --without-afms --enable-binfilter --enable-extensions --enable-minimizer --enable-presenter-console --enable-pdfimport --enable-ogltrans --enable-report-builder --with-sun-templates )
endif
	touch "$@"

build.ooo-build_patches: \
	build.ooo-build_apply_patch \
	build.ooo-build_update_patch \
	build.ooo-build_cws-ooxml03-opc-svx-and-ptt-split.diff_patch
	cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; ./download
	cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; setenv LANG C ; $(MAKE) build.prepare
	touch "$@"

build.oo_patches: \
	build.oo_configure.in_patch \
	build.oo_set_soenv.in_patch \
	build.oo_avmedia_patch \
	build.oo_basic_patch \
	build.oo_bean_patch \
	build.oo_binfilter_patch \
	build.oo_bridges_patch \
	build.oo_connectivity_patch \
	build.oo_cppu_patch \
	build.oo_cppuhelper_patch \
	build.oo_cpputools_patch \
	build.oo_dbaccess_patch \
	build.oo_dtrans_patch \
	build.oo_extensions_patch \
	build.oo_filter_patch \
	build.oo_fpicker_patch \
	build.oo_framework_patch \
	build.oo_i18npool_patch \
	build.oo_idlc_patch \
	build.oo_jfreereport_patch \
	build.oo_jvmaccess_patch \
	build.oo_jvmfwk_patch \
	build.oo_lingucomponent_patch \
	build.oo_lpsolve_patch \
	build.oo_moz_patch \
	build.oo_postprocess_patch \
	build.oo_qadevOOo_patch \
	build.oo_reportbuilder_patch \
	build.oo_reportdesign_patch \
	build.oo_sal_patch \
	build.oo_sc_patch \
	build.oo_scp2_patch \
	build.oo_solenv_patch \
	build.oo_soltools_patch \
	build.oo_sw_patch \
	build.oo_testshl2_patch \
	build.oo_ucb_patch \
	build.oo_ucbhelper_patch \
	build.oo_vcl_patch \
	build.oo_vos_patch \
	build.oo_wizards_patch \
	build.oo_xmlhelp_patch \
	build.oo_xmlsecurity_patch
# Copy modified compiler scripts to force 64 bit compilation of all binaries
	mkdir -p "$(COMPILERDIR)"
	cd "$(COMPILERDIR)" ; sh -c -e 'for i in cc gcc c++ g++ ; do cp "$(PWD)/$(OO_PATCHES_HOME)/cc" "$$i" ; chmod 755 "$$i"; done'
	touch "$@"

build.oo_%.in_patch: $(OO_PATCHES_HOME)/%.in.patch build.ooo-build_patches
	-( cd "$(OOO-BUILD_BUILD_HOME)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(OOO-BUILD_BUILD_HOME)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.oo_bridges_patch: $(OO_PATCHES_HOME)/bridges.patch build.ooo-build_patches
	cd "$(OOO-BUILD_BUILD_HOME)/bridges/source/cpp_uno/gcc3_macosx_intel" ; tar zxvf "$(PWD)/$(OO_PATCHES_HOME)/aoo-4.0.1/bridges_source_cpp_uno_cxx_macosx_x86-64.tar.gz"
	-( cd "$(OOO-BUILD_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(OOO-BUILD_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.oo_moz_patch: $(OO_PATCHES_HOME)/moz.patch build.ooo-build_patches
	cd "$(OOO-BUILD_BUILD_HOME)/moz" ; tar zxvf "$(PWD)/$(OO_PATCHES_HOME)/aoo-4.0.1/nss.tar.gz"
	-( cd "$(OOO-BUILD_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(OOO-BUILD_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	cd "$(OOO-BUILD_BUILD_HOME)/moz/download" ; cp "$(PWD)/$(MOZILLA_PATCHES_HOME)/nss-3.16-with-nspr-4.10.4.tar.gz" .
	touch "$@"

build.oo_%_patch: $(OO_PATCHES_HOME)/%.patch build.ooo-build_patches
ifeq ("$(OS_TYPE)","MacOSX")
	-( cd "$(OOO-BUILD_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(OOO-BUILD_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
else
	-cat "$<" | unix2dos | ( cd "$(OOO-BUILD_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" )
	cat "$<" | unix2dos | ( cd "$(OOO-BUILD_BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" )
endif
	touch "$@"

build.ooo-build_all: build.oo_patches
ifeq ("$(OS_TYPE)","MacOSX")
	cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; "$(MAKE)" PKG_CONFIG="$(PKG_CONFIG)" PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)" build
else
# Copy Visual Studio 9.0 dbghelp.ddl
	sh -e -c 'if [ ! -f "$(OOO-BUILD_BUILD_HOME)/external/dbghelp/dbghelp.dll" ] ; then cp "/cygdrive/c/Program Files/Microsoft Visual Studio 9.0/Common7/IDE/dbghelp.dll" "$(OOO-BUILD_BUILD_HOME)/external/dbghelp/dbghelp.dll" ; fi'
# Prepend Visual Studio 9.0 tools to path
	cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; setenv PATH "/cygdrive/c/Program Files/Microsoft Visual Studio 9.0/VC/bin:$$PATH" ; "$(MAKE)" build
endif
	touch "$@"

build.ooo-build_%_patch: $(OOO-BUILD_PATCHES_HOME)/%.patch build.ooo-build_configure
	-( cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.neo_configure: build.ooo-build_all neo_configure.mk
	$(MAKE) $(MFLAGS) build.neo_configure_phony
	touch "$@"

build.neo_patches: build.ooo-build_all \
	build.remotecontrol_patches \
	$(PRODUCT_COMPONENT_MODULES:%=build.neo_%_component) \
	$(PRODUCT_COMPONENT_PATCH_MODULES:%=build.neo_%_component) \
	build.neo_avmedia_patch \
	build.neo_basic_patch \
	build.neo_canvas_patch \
	build.neo_connectivity_patch \
	build.neo_cppcanvas_patch \
	build.neo_cppuhelper_patch \
	build.neo_cpputools_patch \
	build.neo_dbaccess_patch \
	build.neo_desktop_patch \
	build.neo_drawinglayer_patch \
	build.neo_dtrans_patch \
	build.neo_filter_patch \
	build.neo_formula_patch \
	build.neo_fpicker_patch \
	build.neo_framework_patch \
	build.neo_goodies_patch \
	build.neo_hsqldb_patch \
	build.neo_hwpfilter_patch \
	build.neo_i18npool_patch \
	build.neo_jvmfwk_patch \
	build.neo_lingucomponent_patch \
	build.neo_linguistic_patch \
	build.neo_moz_patch \
	build.neo_oox_patch \
	build.neo_package_patch \
	build.neo_pyuno_patch \
	build.neo_rhino_patch \
	build.neo_reportdesign_patch \
	build.neo_sal_patch \
	build.neo_sax_patch \
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
	build.neo_writerfilter_patch \
	build.neo_xmloff_patch
	touch "$@"

build.neo_%_patch: % build.neo_configure
	rm -Rf "$(PWD)/$</$(UOUTPUTDIR)"
	cd "$<" ; sh -e -c '( cd "$(PWD)/$(OOO-BUILD_BUILD_HOME)/$<" ; find . -type d | sed "s/ /\\ /g" | grep -v /CVS$$ | grep -v /$(UOUTPUTDIR) | grep -v /quicktime ) | while read i ; do mkdir -p "$$i" ; done'
ifeq ("$(OS_TYPE)","MacOSX")
	cd "$<" ; sh -e -c '( cd "$(PWD)/$(OOO-BUILD_BUILD_HOME)/$<" ; find . ! -type d | sed "s/ /\\ /g" | grep -v /CVS/ | grep -v /$(UOUTPUTDIR) | grep -v /quicktime ) | while read i ; do if [ ! -f "$$i" ] ; then ln -sf "$(PWD)/$(OOO-BUILD_BUILD_HOME)/$</$$i" "$$i" 2>/dev/null ; fi ; done'
else
# Use hardlinks for Windows
	cd "$<" ; sh -e -c 'CYGWIN=winsymlinks ; export CYGWIN ; ( cd "$(PWD)/$(OOO-BUILD_BUILD_HOME)/$<" ; find . ! -type d | grep -v /CVS/ | grep -v /$(UOUTPUTDIR) | grep -v /quicktime ) | while read i ; do if [ ! -f "$$i" ] ; then ln -f "$(PWD)/$(OOO-BUILD_BUILD_HOME)/$</$$i" "$$i" 2>/dev/null ; fi ; done'
endif
	source "$(OO_ENV_JAVA)" ; cd "$<" ; `alias build` $(NEO_BUILD_ARGS)
	touch "$@"

build.neo_%_component: % build.neo_configure
	rm -Rf "$(PWD)/$</$(UOUTPUTDIR)"
	mkdir -p "$(PWD)/$</$(UOUTPUTDIR)"
	source "$(OO_ENV_JAVA)" ; cd "$<" ; `alias build` $(NEO_BUILD_ARGS)
	touch "$@"

build.imedia_checkout:
	rm -Rf "$(BUILD_HOME)/$(IMEDIA_PACKAGE)"
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; mkdir "$(IMEDIA_PACKAGE)"
	cd "$(BUILD_HOME)" ; tar zxvf "$(PWD)/$(IMEDIA_PATCHES_HOME)/$(IMEDIA_SOURCE_FILENAME)"
	touch "$@"

build.remotecontrol_checkout:
	rm -Rf "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)"
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; mkdir "$(REMOTECONTROL_PACKAGE)"
	cd "$(BUILD_HOME)" ; tar zxvf "$(PWD)/$(REMOTECONTROL_PATCHES_HOME)/$(REMOTECONTROL_SOURCE_FILENAME)"
	touch "$@"

build.imedia_src_untar: $(IMEDIA_PATCHES_HOME)/additional_source build.imedia_checkout
	cd "$(BUILD_HOME)/$(IMEDIA_PACKAGE)" ; ( cd "$(PWD)/$<" ; tar cf - *.h *.m *.png *.lproj *.plist *.xcodeproj ) | tar xvf -
	touch "$@"

ifeq ("$(OS_TYPE)","MacOSX")
build.imedia_patches: $(IMEDIA_PATCHES_HOME)/imedia.patch build.imedia_src_untar
	-( cd "$(BUILD_HOME)/$(IMEDIA_PACKAGE)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(IMEDIA_PACKAGE)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	cd "$(BUILD_HOME)/$(IMEDIA_PACKAGE)" ; xcodebuild -target iMediaBrowser -configuration Debug clean
	cd "$(BUILD_HOME)/$(IMEDIA_PACKAGE)" ; xcodebuild -target iMediaBrowser -configuration Debug
	touch "$@"
else
build.imedia_patches:
	touch "$@"
endif

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
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	"$(MAKE)" $(MFLAGS) "build.package_shared"
	touch "$@"

build.package2: build.neo_patches
ifndef PRODUCT_BUILD2
	"$(MAKE)" $(MFLAGS) "PRODUCT_BUILD2=TRUE" "PRODUCT_VERSION=$(PRODUCT_VERSION2)" "PRODUCT_VERSION_EXT=$(PRODUCT_VERSION_EXT2)" "CERTAPPIDENTITY=$(CERTAPPIDENTITY2)" "CERTPKGIDENTITY=$(CERTPKGIDENTITY2)" "PRODUCT_BUNDLED_LANG_PACKS=$(PRODUCT_BUNDLED_LANG_PACKS2)" "build.package_shared"
	touch "$@"
endif

build.package_shared:
# Check that codesign and productsign executables exist before proceeding
	@sh -e -c 'for i in codesign productsign ; do if [ -z "`which $$i`" ] ; then echo "$$i command not found" ; exit 1 ; fi ; done'
	sh -e -c 'if [ -d "$(INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(INSTALL_HOME)" ; fi'
	sh -e -c 'if [ -d "/Volumes/OpenOffice.org $(OO_PRODUCT_VERSION_FAMILY)" ] ; then hdiutil eject -force "/Volumes/OpenOffice.org $(OO_PRODUCT_VERSION_FAMILY)" ; fi'
	hdiutil attach -nobrowse "$(OOO-BUILD_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice/dmg/install/en-US/OOo_$(OO_PRODUCT_VERSION)_"*"$(ULONGNAME)_install.dmg"
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
	cd "$(OOO-BUILD_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/install" ; find . -type d -maxdepth 1 -exec basename {} \; | grep -v '^\.$$' | grep -v '^follow_me$$' | grep -v '^log$$' > "$(PWD)/$(INSTALL_HOME)/language_names"
# Include certain languages in the main installer
	sh -e -c 'for i in $(PRODUCT_BUNDLED_LANG_PACKS) ; do if [ -d "/Volumes/OpenOffice.org $(OO_PRODUCT_VERSION_FAMILY)" ] ; then hdiutil eject -force "/Volumes/OpenOffice.org Languagepack" ; fi ; hdiutil attach -nobrowse "$(PWD)/$(OOO-BUILD_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/install/$${i}/OpenOffice.org-langpack-$(OO_PRODUCT_VERSION)_$${i}.dmg" ; bunzip2 -dc "/Volumes/OpenOffice.org Languagepack/OpenOffice.org Languagepack.app/Contents/tarball.tar.bz2" | ( cd "$(PWD)/$(INSTALL_HOME)/package" ; gnutar xvf - --exclude="._*" ) ; hdiutil eject -force "/Volumes/OpenOffice.org Languagepack" ; helpflag=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | awk -F, "{ print \\$$2 }"` ; if [ "$${helpflag}" != "1" ] ; then rm -Rf "$(PWD)/$(INSTALL_HOME)/package/Contents/help/$${i}" ; ( cd "$(PWD)/$(INSTALL_HOME)/package/Contents/help" ; ln -s "en" "$${i}" ) ; fi ; done'
ifndef LANGPACKS
# Bypass the language pack installers
else
# Create the language pack installers
	sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names" | sed $(foreach BUNDLED_LANG_PACK,$(PRODUCT_BUNDLED_LANG_PACKS),-e "/^$(BUNDLED_LANG_PACK)\\$$/d")` ; do langname=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | sed "s/#.*$$//" | awk -F, "{ print \\$$3 }"` ; langdirname=`echo "$${langname}" | sed "s# #_#g"` ; if [ -z "$${langname}" -o -z "$${langdirname}" ] ; then echo "Skipping $${i} language..." ; continue ; fi ; mkdir -p "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents" ; if [ -d "/Volumes/OpenOffice.org $(OO_PRODUCT_VERSION_FAMILY)" ] ; then hdiutil eject -force "/Volumes/OpenOffice.org Languagepack" ; fi ; hdiutil attach -nobrowse "$(PWD)/$(OOO-BUILD_BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/install/$${i}/OpenOffice.org-langpack-$(OO_PRODUCT_VERSION)_$${i}.dmg" ; bunzip2 -dc "/Volumes/OpenOffice.org Languagepack/OpenOffice.org Languagepack.app/Contents/tarball.tar.bz2" | ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}" ; gnutar xvf - --exclude="._*" ) ; hdiutil eject -force "/Volumes/OpenOffice.org Languagepack" ; rm -f "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/MacOS/resource/ooo"*.res "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/basis-link/program/resource/dba"*.res ; helpflag=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | awk -F, "{ print \\$$2 }"` ; if [ "$${helpflag}" != "1" ] ; then rm -Rf "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help/$${i}" ; ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help" ; ln -s "en" "$${i}" ) ; fi ; "$(MAKE)" $(MFLAGS) "PRODUCT_LANG_PACK_LOCALE=$${i}" "PRODUCT_LANG_PACK_VERSION=$(PRODUCT_LANG_PACK_VERSION) $${langname}" "PRODUCT_DIR_LANG_PACK_VERSION=$(PRODUCT_DIR_LANG_PACK_VERSION)_$${langdirname}" "build.package_$${langdirname}" ; done'
endif
	chmod -Rf u+w,a+r "$(INSTALL_HOME)/package"
	cd "$(INSTALL_HOME)/package/Contents" ; mv -f "MacOS/resource" "etc" ; ln -sf "../etc/resource" "MacOS/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/avmedia/$(UOUTPUTDIR)/lib/libavmedia$(DLLSUFFIX).dylib" "$(PWD)/avmedia/$(UOUTPUTDIR)/lib/libavmediaquicktime.dylib" "$(PWD)/basic/$(UOUTPUTDIR)/lib/libsb$(DLLSUFFIX).dylib" "$(PWD)/canvas/$(UOUTPUTDIR)/lib/vclcanvas.uno.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libcalc$(DLLSUFFIX).dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libdbase$(DLLSUFFIX).dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libdbtools$(DLLSUFFIX).dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libmacab1.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libmacabdrv1.dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libodbc$(DLLSUFFIX).dylib" "$(PWD)/connectivity/$(UOUTPUTDIR)/lib/libodbcbase$(DLLSUFFIX).dylib" "$(PWD)/cppcanvas/$(UOUTPUTDIR)/lib/libcppcanvas$(DLLSUFFIX).dylib" "$(PWD)/dbaccess/$(UOUTPUTDIR)/lib/libdba$(DLLSUFFIX).dylib" "$(PWD)/dbaccess/$(UOUTPUTDIR)/lib/libdbu$(DLLSUFFIX).dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/deployment$(DLLSUFFIX).uno.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/deploymentgui$(DLLSUFFIX).uno.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libdeploymentmisc$(DLLSUFFIX).dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libsofficeapp.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libspl$(DLLSUFFIX).dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/libunopkgapp.dylib" "$(PWD)/desktop/$(UOUTPUTDIR)/lib/migrationoo2.uno.dylib" "$(PWD)/drawinglayer/$(UOUTPUTDIR)/lib/libdrawinglayer$(DLLSUFFIX).dylib" "$(PWD)/dtrans/$(UOUTPUTDIR)/lib/libdtransjava$(DLLSUFFIX).dylib" "$(PWD)/extensions/$(UOUTPUTDIR)/lib/libscn$(DLLSUFFIX).dylib" "$(PWD)/filter/$(UOUTPUTDIR)/lib/libpdffilter$(DLLSUFFIX).dylib" "$(PWD)/formula/$(UOUTPUTDIR)/lib/libfor$(DLLSUFFIX).dylib" "$(PWD)/fpicker/$(UOUTPUTDIR)/lib/fpicker.uno.dylib" "$(PWD)/fpicker/$(UOUTPUTDIR)/lib/fps_java.uno.dylib" "$(PWD)/framework/$(UOUTPUTDIR)/lib/libfwe$(DLLSUFFIX).dylib" "$(PWD)/framework/$(UOUTPUTDIR)/lib/libfwk$(DLLSUFFIX).dylib" "$(PWD)/goodies/$(UOUTPUTDIR)/lib/libgo$(DLLSUFFIX).dylib" "$(PWD)/goodies/$(UOUTPUTDIR)/lib/libipt$(DLLSUFFIX).dylib" "$(PWD)/hwpfilter/$(UOUTPUTDIR)/lib/libhwp.dylib" "$(PWD)/i18npool/$(UOUTPUTDIR)/lib/i18npool.uno.dylib" "$(PWD)/lingucomponent/$(UOUTPUTDIR)/lib/libspell$(DLLSUFFIX).dylib" "$(PWD)/linguistic/$(UOUTPUTDIR)/lib/liblng$(DLLSUFFIX).dylib" "$(PWD)/moz/$(UOUTPUTDIR)/lib/libfreebl3.dylib" "$(PWD)/moz/$(UOUTPUTDIR)/lib/libsoftokn3.dylib" "$(PWD)/oox/$(UOUTPUTDIR)/lib/liboox$(DLLSUFFIX).dylib" "$(PWD)/package/$(UOUTPUTDIR)/lib/libpackage2.dylib" "$(PWD)/package/$(UOUTPUTDIR)/lib/libxstor.dylib" "$(PWD)/reportdesign/$(UOUTPUTDIR)/lib/librpt$(DLLSUFFIX).dylib" "$(PWD)/sax/$(UOUTPUTDIR)/lib/fastsax.uno.dylib" "$(PWD)/sc/$(UOUTPUTDIR)/lib/libsc$(DLLSUFFIX).dylib" "$(PWD)/sc/$(UOUTPUTDIR)/lib/libscui$(DLLSUFFIX).dylib" "$(PWD)/sd/$(UOUTPUTDIR)/lib/libpptx$(DLLSUFFIX).dylib" "$(PWD)/sd/$(UOUTPUTDIR)/lib/libsd$(DLLSUFFIX).dylib" "$(PWD)/sd/$(UOUTPUTDIR)/lib/libsdui$(DLLSUFFIX).dylib" "$(PWD)/sfx2/$(UOUTPUTDIR)/lib/libsfx$(DLLSUFFIX).dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/cmdmail.uno.dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/localebe1.uno.dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/macbe1.uno.dylib" "$(PWD)/shell/$(UOUTPUTDIR)/lib/syssh.uno.dylib" "$(PWD)/sot/$(UOUTPUTDIR)/lib/libsot$(DLLSUFFIX).dylib" "$(PWD)/svtools/$(UOUTPUTDIR)/lib/libsvl$(DLLSUFFIX).dylib" "$(PWD)/svtools/$(UOUTPUTDIR)/lib/libsvt$(DLLSUFFIX).dylib" "$(PWD)/svx/$(UOUTPUTDIR)/lib/libcui$(DLLSUFFIX).dylib" "$(PWD)/svx/$(UOUTPUTDIR)/lib/libsvx$(DLLSUFFIX).dylib" "$(PWD)/svx/$(UOUTPUTDIR)/lib/libsvxcore$(DLLSUFFIX).dylib" "$(PWD)/svx/$(UOUTPUTDIR)/lib/libsvxmsfilter$(DLLSUFFIX).dylib" "$(PWD)/sw/$(UOUTPUTDIR)/lib/libdocx$(DLLSUFFIX).dylib" "$(PWD)/sw/$(UOUTPUTDIR)/lib/libmsword$(DLLSUFFIX).dylib" "$(PWD)/sw/$(UOUTPUTDIR)/lib/libsw$(DLLSUFFIX).dylib" "$(PWD)/sw/$(UOUTPUTDIR)/lib/libswui$(DLLSUFFIX).dylib" "$(PWD)/tools/$(UOUTPUTDIR)/lib/libtl$(DLLSUFFIX).dylib" "$(PWD)/ucb/$(UOUTPUTDIR)/lib/libucpdav1.dylib" "$(PWD)/ucb/$(UOUTPUTDIR)/lib/libucpfile1.dylib" "$(PWD)/ucbhelper/$(UOUTPUTDIR)/lib/libucbhelper4gcc3.dylib" "$(PWD)/unotools/$(UOUTPUTDIR)/lib/libutl$(DLLSUFFIX).dylib" "$(PWD)/unoxml/$(UOUTPUTDIR)/lib/libunordf$(DLLSUFFIX).dylib" "$(PWD)/unoxml/$(UOUTPUTDIR)/lib/libunoxml$(DLLSUFFIX).dylib" "$(PWD)/uui/$(UOUTPUTDIR)/lib/libuui$(DLLSUFFIX).dylib" "$(PWD)/vcl/$(UOUTPUTDIR)/lib/libvcl$(DLLSUFFIX).dylib" "$(PWD)/vos/$(UOUTPUTDIR)/lib/libvos3gcc3.dylib" "$(PWD)/writerfilter/$(UOUTPUTDIR)/lib/libwriterfilter$(DLLSUFFIX).dylib" "$(PWD)/writerperfect/$(UOUTPUTDIR)/lib/libmsworks$(DLLSUFFIX).dylib" "$(PWD)/writerperfect/$(UOUTPUTDIR)/lib/libwpft$(DLLSUFFIX).dylib" "$(PWD)/writerperfect/$(UOUTPUTDIR)/lib/libwpgimport$(DLLSUFFIX).dylib" "$(PWD)/xmloff/$(UOUTPUTDIR)/lib/libxo$(DLLSUFFIX).dylib" "basis-link/program"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/cppuhelper/$(UOUTPUTDIR)/lib/libuno_cppuhelpergcc3.dylib.3" "$(PWD)/jvmfwk/$(UOUTPUTDIR)/lib/libjvmfwk.dylib.3" "$(PWD)/sal/$(UOUTPUTDIR)/lib/libuno_sal.dylib.3" "$(PWD)/store/$(UOUTPUTDIR)/lib/libstore.dylib.3" "basis-link/ure-link/lib"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/vcl/$(UOUTPUTDIR)/bin/salapp"*.res "etc/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/cpputools/$(UOUTPUTDIR)/bin/uno" "basis-link/ure-link/bin/uno.bin" ; chmod a+x "basis-link/ure-link/bin/uno.bin"
ifdef PRODUCT_BUILD2
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice2" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
else
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
endif
# Mac App Store will reject apps with shell scripts
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in sbase scalc sdraw simpress smath soffice swriter unopkg unopkg.bin ; do rm -f "MacOS/$$i" ; ln -sf "soffice.bin" "MacOS/$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "basis-link/program/gengal" ; ln -sf "gengal.bin" "basis-link/program/gengal"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in regcomp uno ; do rm -f "basis-link/ure-link/bin/$$i" ; ln -sf "$$i.bin" "basis-link/ure-link/bin/$$i" ; done'
ifdef PRODUCT_BUILD2
	cd "$(INSTALL_HOME)/package/Contents" ; cp -f "$(PWD)/sfx2/$(UOUTPUTDIR)/bin/objserv_cocoa"*.res "etc/resource"
endif
	cd "$(INSTALL_HOME)/package/Contents" ; cp -f "$(PWD)/sfx2/$(UOUTPUTDIR)/bin/shutdowniconjava"*.res "etc/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp -f "$(PWD)/sw/$(UOUTPUTDIR)/bin/swmacdictlookup"*.res "etc/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "etc/resource/ooo"*.res ; cp -f "$(PWD)/svx/$(UOUTPUTDIR)/bin/ooo"*.res "etc/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "basis-link/program/resource/dba"*.res ; cp -f "$(PWD)/dbaccess/$(UOUTPUTDIR)/bin/dba"*.res "basis-link/program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "basis-link/program/resource/deploymentgui"*.res ; cp -f "$(PWD)/desktop/$(UOUTPUTDIR)/bin/deploymentgui"*.res "basis-link/program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in editpic.pl poll.pl savepic.pl show.pl ; do rm -f "basis-link/share/config/webcast/$$i" ; cp -f "$(PWD)/sd/res/webview/$$i" "basis-link/share/config/webcast/$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "basis-link/share/config/webcast/webcast.pl" ; cp -f "$(PWD)/sd/res/webview/webview.pl" "basis-link/share/config/webcast/webcast.pl"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/shell/$(UOUTPUTDIR)/bin/senddoc" "basis-link/program/senddoc" ; chmod a+x "basis-link/program/senddoc"
# Set build version using date and hour. Mac App Store limits to 10 digits.
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/package/Info.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME)#$(PRODUCT_TRADEMARKED_NAME)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(PRODUCT_FILETYPE)#$(PRODUCT_FILETYPE)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(PRODUCT_BUILD_VERSION)#$(PRODUCT_VERSION).'"`date -u '+%Y%m%d%H'`"'#g' > "Info.plist"
	cd "$(INSTALL_HOME)/package/Contents" ; printf '%s' 'APPL$(PRODUCT_FILETYPE)' > "PkgInfo"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#^ProgressPosition=.*$$#ProgressPosition=14,260#g' "MacOS/sofficerc" > "etc/sofficerc" ; rm -f "MacOS/sofficerc" ; ln -sf "../etc/sofficerc" "MacOS/sofficerc"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in bootstraprc fundamentalrc setuprc versionrc ; do mv -f "MacOS/$$i" "etc/$$i" ; ln -sf "../etc/$$i" "MacOS/$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find "basis-link/share/config/soffice.cfg/modules" -name "menubar.xml"` ; do sed "s#<menu:menuitem.*\.uno:TwainSelect.*/>#<\!--&-->#g" "$${i}" > "../../out" ; mv -f "../../out" "$${i}" ; done'
	rm -Rf "$(INSTALL_HOME)/package/Contents/Resources"
	mkdir -p "$(INSTALL_HOME)/package/Contents/Resources/cursors"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/package/ship.icns" "Resources"
endif
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
ifdef PRODUCT_BUILD2
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "MacOS/intro.bmp" ; cp "$(PWD)/etc/package/intro_free.bmp" "etc/intro.bmp" ; ln -sf "../etc/intro.bmp" "MacOS/intro.bmp"
else
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "MacOS/intro.bmp" ; cp "tmp/NeoOffice Aqua Elements 3/Contents/MacOS/intro.bmp" "etc/intro.bmp" ; ln -sf "../etc/intro.bmp" "MacOS/intro.bmp"
endif
endif
	cd "$(INSTALL_HOME)/package/Contents" ; cp "tmp/NeoOffice Aqua Elements 3/Contents/Resources/"*.icns "Resources"
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"` svtools svx
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"` svtools svx
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 3/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libAppleRemote$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libMacOSXSpell$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libavmediaQuickTime$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libavmediaquicktime.dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libdtransaqua$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libdtransjava$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/fps_aqua.uno.dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/fps_java.uno.dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/updchk.uno.dylib'
# Do not ship the Lotus Word Pro filter as it is very unstable on Mac OS X
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/liblwpft$(DLLSUFFIX).dylib'
# Reregister hwpfilter
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libhwp.dylib'
	source "$(OO_ENV_JAVA)" ; setenv DYLD_LIBRARY_PATH "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/program:$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/lib:$$DYLD_LIBRARY_PATH" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/ure-link/bin/regcomp" -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libhwp.dylib'
# Reregister spellchecker to enable native grammarchecker
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libspell$(DLLSUFFIX).dylib'
# Add Mac OS X localized resources
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; sh -e -c 'for i in `echo "$(PRODUCT_BUNDLED_LANG_PACKS)" | sed "s#-#_#g"` ; do mkdir -p "$${i}.lproj" ; mkdir -p `echo "$${i}" | sed "s#_.*\\$$##"`".lproj" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "MacOS/unoinfo" "basis-link/program/open-url" "basis-link/program/senddoc" LICENSE* README* basis-link/licenses basis-link/readmes "basis-link/ure-link/bin/startup.sh" share/readme
# Fix bug 3273 by not installing any OOo or ooo-build fonts
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "basis-link/program/libAppleRemote$(DLLSUFFIX).dylib" "basis-link/program/libMacOSXSpell$(DLLSUFFIX).dylib" "basis-link/program/libavmediaQuickTime$(DLLSUFFIX).dylib" "basis-link/program/libdtransaqua$(DLLSUFFIX).dylib" "basis-link/program/fps_aqua.uno.dylib" "basis-link/program/liblwpft$(DLLSUFFIX).dylib" "basis-link/share/fonts/truetype" "basis-link/share/psprint"
# Remove all update check files since the Mac App Store has its own update
# check. Do not remove updatefeed.uno.dylib as it is needed by the pdfimport
# extension.
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "basis-link/program/libupdchk$(DLLSUFFIX).dylib" "basis-link/share/registry/data/org/openoffice/Office/Addons.xcu" "basis-link/share/registry/data/org/openoffice/Office/Jobs.xcu"
	cd "$(INSTALL_HOME)/package/Contents/basis-link" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share -type f | grep -v /CVS | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents/basis-link" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share -type f | grep -v /CVS` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
# Remove report toolbar from Base since reports require Java support
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find "basis-link/share/registry" -name BaseWindowState.xcu` ; do sed "s#\"private:resource\/toolbar\/reportobjectbar\"#\"ignore\"#g" "$${i}" > "../../out" ; mv -f "../../out" "$${i}" ; done'
	rm "$(INSTALL_HOME)/package/Contents/basis-link/share/config/soffice.cfg/modules/dbapp/toolbar/reportobjectbar.xml"
	cd "$(INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/$(PRODUCT_DIR_NAME)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' | sed 's#BuildMachine=.*$$#BuildMachine=$(BUILD_MACHINE)#g' > "../../out" ; mv -f "../../out" "etc/bootstraprc"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/program/fundamentalrc" "etc/fundamentalrc"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/program/versionrc" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_UPDATE_CHECK_URL)#$(PRODUCT_UPDATE_CHECK_URL)#g' | sed 's# #%20#g' | sed 's#^buildid=.*$$#buildid=$(PRODUCT_PATCH_VERSION)#' > "etc/versionrc"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in "share/registry/data/org/openoffice/Office/Compatibility.xcu" "share/registry/modules/org/openoffice/Office/Common/Common-brand.xcu" "share/registry/modules/org/openoffice/Office/UI/UI-brand.xcu" "share/registry/modules/org/openoffice/Setup/Setup-brand.xcu" ; do sed "s#oor:name=\"$(OO_PRODUCT_NAME)\"#oor:name=\"$(PRODUCT_NAME)\"#g" "$${i}" | sed "s#>$(OO_PRODUCT_NAME)<#>$(PRODUCT_NAME)<#g" | sed "s#>$(OO_PRODUCT_VERSION_FAMILY)<#>$(PRODUCT_VERSION)<#g" | sed "s#>$(OO_PRODUCT_VERSION)<#>$(PRODUCT_VERSION)<#g" | sed "s#>$(OO_REGISTRATION_URL)<#>$(PRODUCT_REGISTRATION_URL)<#g" > "../../out" ; mv -f "../../out" "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(OO_PRODUCT_NAME)#$(OO_PRODUCT_NAME)#g' "$(PWD)/etc/help/main_transform.xsl" | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL)#$(PRODUCT_DOCUMENTATION_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL_TEXT)#$(PRODUCT_DOCUMENTATION_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#g' > "basis-link/help/main_transform.xsl"
# With gcc 4.x, we must fully strip executables
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.bin"` ; do strip "$$i" ; done'
# Mac OS 10.5.x and higher cannot strip the Mozilla libraries to exclude them
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*" | grep -v "components"` ; do strip -S -x "$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.so"` ; do strip -S -x "$$i" ; done'
# Integrate the RemoteControl framework
	mkdir -p "$(INSTALL_HOME)/package/Contents/Frameworks"
	cd "$(INSTALL_HOME)/package" ; ( ( cd "$(PWD)/$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)/build/Release" ; gnutar cvf - --exclude Headers RemoteControl.framework ) | ( cd "$(PWD)/$(INSTALL_HOME)/package/Contents/Frameworks" ; gnutar xvf - ; strip -S -x RemoteControl.framework/Versions/A/RemoteControl ) )
# Install OOo .oxt files. Note that we exclude the wiki-publisher.oxt file as
# has been found to have buggy network connectivity. Fix crashing bug by using
# our patched build of pdfimport.oxt. Fix bug 3501 by using our patched build
# of sun-presentation-minimizer.oxt.
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -c -e 'JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1 ; export JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY ; unset CLASSPATH ; unset DYLD_LIBRARY_PATH ; for i in `find "$(PWD)/$(OOO-BUILD_BUILD_HOME)/solver/$${UPD}/$(UOUTPUTDIR)/bin" -type f -name "*.oxt" | grep -v "wiki-publisher.oxt" | grep -v "pdfimport.oxt" | grep -v "sun-presentation-minimizer.oxt"` "$(PWD)/sdext/$(UOUTPUTDIR)/bin/pdfimport.oxt" "$(PWD)/sdext/$(UOUTPUTDIR)/bin/sun-presentation-minimizer.oxt" ; do rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp" ; echo "yes" | ./unopkg.bin add --shared --verbose "$$i" -env:UserInstallation=file://"$(PWD)/$(INSTALL_HOME)/tmp" ; done ; rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp"'
# Reinstall dictionary .oxt files removed by patches/openoffice/scp2.patch
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -c -e 'JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1 ; export JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY ; unset CLASSPATH ; unset DYLD_LIBRARY_PATH ; for i in `find -E "$(PWD)/$(OOO-BUILD_BUILD_HOME)/solver/$${UPD}/$(UOUTPUTDIR)/pck" -type f -name "*.oxt" -regex ".*\/dict-(en|es|fr)\.oxt"` ; do rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp" ; echo "yes" | ./unopkg.bin add --shared --verbose "$$i" -env:UserInstallation=file://"$(PWD)/$(INSTALL_HOME)/tmp" ; done ; rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp"'
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
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type f -name "*.bin" \! -name "soffice.bin"` "Contents/basis-link/program/msfontextract" "Contents/basis-link/program/pagein" "Contents/basis-link/program/uri-encode" "Contents/basis-link/ure-link/bin/regmerge" "Contents/basis-link/ure-link/bin/regview" `find "Contents/share/uno_packages/cache/uno_packages" -type f -name "xpdfimport"` ; do codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/etc/package/Entitlements_inherit_only.plist" "$$i" ; done'
	cat "etc/package/Entitlements.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(INSTALL_HOME)/Entitlements.plist"
	cd "$(INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/$(INSTALL_HOME)/Entitlements.plist" .
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
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/PackageInfo" | sed 's#$$(PRODUCT_INSTALL_DIR_NAME)#$(PRODUCT_INSTALL_DIR_NAME)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Distribution" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(TARGET_MACHINE)#$(TARGET_MACHINE)#g' > "$(INSTALL_HOME)/package.pkg/Distribution"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cp "etc/package/ship.tiff" "$(INSTALL_HOME)/package.pkg/Resources/background.tiff"
	echo '<background file="background.tiff" alignment="bottomleft" scaling="proportional"/>' >> "$(INSTALL_HOME)/package.pkg/Distribution"
endif
	mkbom "$(INSTALL_HOME)/package" "$(INSTALL_HOME)/package.pkg/contents.pkg/Bom" >& /dev/null
	( cd "$(INSTALL_HOME)/package" ; pax -w -z -x cpio . ) > "$(INSTALL_HOME)/package.pkg/contents.pkg/Payload"
ifdef PRODUCT_BUILD2
	echo '<scripts><postinstall file="./postflight"/></scripts>' >> "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
endif
	echo '<payload installKBytes="'`du -sk "$(INSTALL_HOME)/package" | awk '{ print $$1 }'`'" numberOfFiles="'`lsbom "$(INSTALL_HOME)/package.pkg/contents.pkg/Bom" | wc -l`'"/>' >> "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	echo '</pkg-info>' >> "$(INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	cat "bin/InstallationCheck.strings" | sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(PREFLIGHT_REQUIRED_COMMANDS)#g' > "$(INSTALL_HOME)/package.pkg/Resources/Localizable.strings"
	cd "$(INSTALL_HOME)/package.pkg/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../Localizable.strings" "$${i}/Localizable.strings" ; done'
ifdef PRODUCT_BUILD2
	mkdir -p "$(INSTALL_HOME)/package.pkg/contents.pkg/Scripts"
	cat "bin/postflight" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' > "$(INSTALL_HOME)/package.pkg/contents.pkg/Scripts/postflight" ; chmod a+x "$(INSTALL_HOME)/package.pkg/contents.pkg/Scripts/postflight"
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
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	"$(MAKE)" $(MFLAGS) "build.patch_package_shared"
	touch "$@"

build.patch_package2: build.package2
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
ifndef PRODUCT_BUILD2
	"$(MAKE)" $(MFLAGS) "PRODUCT_BUILD2=TRUE" "PRODUCT_VERSION=$(PRODUCT_VERSION2)" "PRODUCT_VERSION_EXT=$(PRODUCT_VERSION_EXT2)" "CERTAPPIDENTITY=$(CERTAPPIDENTITY2)" "CERTPKGIDENTITY=$(CERTPKGIDENTITY2)" "PRODUCT_BUNDLED_LANG_PACKS=$(PRODUCT_BUNDLED_LANG_PACKS2)" "build.patch_package_shared"
	touch "$@"
endif

build.patch_package_shared:
# Check that codesign and productsign executables exist before proceeding
	@sh -e -c 'for i in codesign productsign ; do if [ -z "`which $$i`" ] ; then echo "$$i command not found" ; exit 1 ; fi ; done'
	sh -e -c 'if [ -d "$(PATCH_INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(PATCH_INSTALL_HOME)" ; fi'
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/MacOS"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/basis-link/program"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/etc"
# Copy all resource files in the main installer and overwrite newer resources
# so that the codesigning will not remove resource files marked as signed in an
# existing installation
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/Resources"
	cd "$(PATCH_INSTALL_HOME)/package/Contents/Resources" ; ( ( cd "$(PWD)/$(INSTALL_HOME)/package/$(PRODUCT_INSTALL_DIR_NAME).app/Contents/Resources" ; find . \! -type d -print0 | xargs -0 gnutar cvf - ) | gnutar xvf - );
ifdef PRODUCT_BUILD2
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice2" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
else
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/desktop/$(UOUTPUTDIR)/bin/soffice" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
endif
	chmod -Rf u+w,a+r "$(PATCH_INSTALL_HOME)/package"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/vcl/$(UOUTPUTDIR)/lib/libvcl$(DLLSUFFIX).dylib" "basis-link/program"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/package/Info.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME)#$(PRODUCT_TRADEMARKED_NAME)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(PRODUCT_FILETYPE)#$(PRODUCT_FILETYPE)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(PRODUCT_BUILD_VERSION)#$(PRODUCT_VERSION)#g' > "Info.plist"
	cd "$(PATCH_INSTALL_HOME)/package/Contents/basis-link" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share -type f | grep -v /CVS | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents/basis-link" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share -type f | grep -v /CVS` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/$(PRODUCT_DIR_NAME)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' | sed 's#BuildMachine=.*$$#BuildMachine=$(BUILD_MACHINE)#g' > "../../out" ; mv -f "../../out" "etc/bootstraprc"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/program/versionrc" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_UPDATE_CHECK_URL)#$(PRODUCT_UPDATE_CHECK_URL)#g' | sed 's# #%20#g' | sed 's#^buildid=.*$$#buildid=$(PRODUCT_PATCH_VERSION)#' > "etc/versionrc"
# With gcc 4.x, we must fully strip executables
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.bin"` ; do strip "$$i" ; done'
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
	cd "$(PATCH_INSTALL_HOME)/package" ; sh -e -c 'for i in `find . -type f -name "*.bin" \! -name "soffice.bin"` ; do codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/etc/package/Entitlements_inherit_only.plist" "$$i" ; done'
	cat "etc/package/Entitlements.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(PATCH_INSTALL_HOME)/Entitlements.plist"
	cd "$(PATCH_INSTALL_HOME)/package" ; codesign --force -s "$(CERTAPPIDENTITY)" --entitlements "$(PWD)/$(PATCH_INSTALL_HOME)/Entitlements.plist" .
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
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/PackageInfo.patch" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/PackageInfo"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Distribution.patch" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_DIR_PATCH_VERSION_EXTRA)#$(PRODUCT_DIR_PATCH_VERSION_EXTRA)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(TARGET_MACHINE)#$(TARGET_MACHINE)#g' > "$(PATCH_INSTALL_HOME)/package.pkg/Distribution"
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
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils.patch" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(OO_PRODUCT_VERSION_FAMILY)#$(OO_PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION_BASE)#$(PRODUCT_VERSION_BASE)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PREVIOUS_PRODUCT_VERSION)#$(PREVIOUS_PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(TARGET_FILE_TYPE)#$(TARGET_FILE_TYPE)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(PATCH_INSTALL_HOME)/installutils"
	cat "$(PATCH_INSTALL_HOME)/installutils" "bin/InstallationCheck" | sed 's#$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g' > "$(PATCH_INSTALL_HOME)/InstallationCheck" ; chmod a+x "$(PATCH_INSTALL_HOME)/InstallationCheck"
	cat "$(PATCH_INSTALL_HOME)/installutils" "bin/VolumeCheck.patch" > "$(PATCH_INSTALL_HOME)/VolumeCheck" ; chmod a+x "$(PATCH_INSTALL_HOME)/VolumeCheck"
	cat "bin/InstallationCheck.strings" "bin/VolumeCheck.strings.patch" | sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g' > "$(PATCH_INSTALL_HOME)/package.pkg/Resources/Localizable.strings"
	cd "$(PATCH_INSTALL_HOME)/package.pkg/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../Localizable.strings" "$${i}/Localizable.strings" ; done'
	sh -e "etc/convertscripttojsstring.sh" "$(PATCH_INSTALL_HOME)/InstallationCheck" > "$(PATCH_INSTALL_HOME)/InstallationCheck.js"
	sh -e "etc/convertscripttojsstring.sh" "$(PATCH_INSTALL_HOME)/VolumeCheck" > "$(PATCH_INSTALL_HOME)/VolumeCheck.js"
	sed 's#$$(PRODUCT_NAME_AND_VERSION)#$(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION)#g' "etc/Distribution.js" | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g' | sh -e -c 'sed "s#var.*installationCheckBashScript.*=.*;#var installationCheckBashScript = unescape(\""`cat "$(PATCH_INSTALL_HOME)/InstallationCheck.js"`"\");#" | sed "s#var.*volumeCheckBashScript.*=.*;#var volumeCheckBashScript = unescape(\""`cat "$(PATCH_INSTALL_HOME)/VolumeCheck.js"`"\");#"' >> "$(PATCH_INSTALL_HOME)/package.pkg/Distribution"
	cat "$(PATCH_INSTALL_HOME)/installutils" "bin/preflight.patch" > "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Scripts/preflight" ; chmod a+x "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Scripts/preflight"
	cat "$(PATCH_INSTALL_HOME)/installutils" "bin/postflight.patch" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Scripts/postflight" ; chmod a+x "$(PATCH_INSTALL_HOME)/package.pkg/contents.pkg/Scripts/postflight"
	mkdir -p "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
	echo '</installer-gui-script>' >> "$(PATCH_INSTALL_HOME)/package.pkg/Distribution"
	pkgutil --flatten "$(PATCH_INSTALL_HOME)/package.pkg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION) $(PRODUCT_DIR_PATCH_VERSION_EXTRA).pkg"
# Sign package
	mv -f "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION) $(PRODUCT_DIR_PATCH_VERSION_EXTRA).pkg" "$(PATCH_INSTALL_HOME)/unsigned.pkg"
	productsign --sign "$(CERTPKGIDENTITY)" "$(PATCH_INSTALL_HOME)/unsigned.pkg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION) $(PRODUCT_DIR_PATCH_VERSION_EXTRA).pkg"
	rm -f "$(PATCH_INSTALL_HOME)/unsigned.pkg"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	rm -Rf "$(PATCH_INSTALL_HOME)/tmp"
	mkdir -p "$(PATCH_INSTALL_HOME)/tmp"
	cd "$(PATCH_INSTALL_HOME)/tmp" ; unzip "$(PWD)/etc/package/SetFileIcon.zip" ; $(CC) -o "SetFileIcon/SetFileIcon" -framework AppKit "SetFileIcon/SetFileIcon.m" ; "SetFileIcon/SetFileIcon" -image "$(PWD)/etc/package/ship.icns" -file "$(PWD)/$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)/Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION) $(PRODUCT_DIR_PATCH_VERSION_EXTRA).pkg"
	rm -Rf "$(PATCH_INSTALL_HOME)/tmp"
endif
	chmod -Rf a-w,a+r "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
	chmod -f u+w "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
# Use YourSway fancy .dmg tool to create .dmg file
	rm -Rf "$(PATCH_INSTALL_HOME)/tmp"
	mkdir -p "$(PATCH_INSTALL_HOME)/tmp"
	cd "$(PATCH_INSTALL_HOME)/tmp" ; unzip "$(PWD)/etc/package/$(YOURSWAYCREATEDMG_SOURCE_FILENAME)"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	sync ; "$(PATCH_INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" --volname "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION) $(PRODUCT_DIR_PATCH_VERSION_EXTRA)" --volicon "etc/package/ship.icns" --icon-size 128 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION) $(PRODUCT_DIR_PATCH_VERSION_EXTRA).pkg" 250 100 --icon "Install $(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_PATCH_VERSION) $(PRODUCT_DIR_PATCH_VERSION_EXTRA).pkg" 250 100 --window-pos 400 300 --window-size 500 250 "$(PATCH_INSTALL_HOME)/$(PRODUCT_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).dmg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
else
	sync ; "$(PATCH_INSTALL_HOME)/tmp/$(YOURSWAYCREATEDMG_PACKAGE)/create-dmg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).dmg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
endif
	rm -Rf "$(PATCH_INSTALL_HOME)/tmp"

build.package_%: $(INSTALL_HOME)/package_%
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	chmod -Rf u+w,a+r "$<"
	cd "$</Contents" ; rm -Rf basis-link/licenses basis-link/readmes share/readme
# Remove report toolbar from Base since reports require Java support
	cd "$</Contents" ; sh -e -c 'for i in `find "basis-link/share/registry" -name BaseWindowState.xcu` ; do sed "s#\"private:resource\/toolbar\/reportobjectbar\"#\"ignore\"#g" "$${i}" > "../../out" ; mv -f "../../out" "$${i}" ; done'
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
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/PackageInfo.langpack" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_LANG_PACK_VERSION)#$(PRODUCT_LANG_PACK_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$<.pkg/contents.pkg/PackageInfo"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Distribution.langpack" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_LANG_PACK_VERSION)#$(PRODUCT_LANG_PACK_VERSION)#g' | sed 's#$$(PRODUCT_SHORT_VERSION)#$(PRODUCT_SHORT_VERSION)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' | sed 's#$$(TARGET_MACHINE)#$(TARGET_MACHINE)#g' > "$<.pkg/Distribution"
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
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils.langpack" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(OO_PRODUCT_VERSION_FAMILY)#$(OO_PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION_BASE)#$(PRODUCT_VERSION_BASE)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_VERSION_EXT)#$(PRODUCT_VERSION_EXT)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PREVIOUS_PRODUCT_VERSION_BASE)#$(PREVIOUS_PRODUCT_VERSION_BASE)#g' | sed 's#$$(TARGET_FILE_TYPE)#$(TARGET_FILE_TYPE)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$(INSTALL_HOME)/installutils.langpack"
	cat "$(INSTALL_HOME)/installutils.langpack" "bin/InstallationCheck" | sed 's#$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g' > "$(INSTALL_HOME)/InstallationCheck.langpack" ; chmod a+x "$(INSTALL_HOME)/InstallationCheck.langpack"
	cat "$(INSTALL_HOME)/installutils.langpack" "bin/VolumeCheck.langpack" > "$(INSTALL_HOME)/VolumeCheck.langpack" ; chmod a+x "$(INSTALL_HOME)/VolumeCheck.langpack"
	cat "bin/InstallationCheck.strings" "bin/VolumeCheck.strings.langpack" | sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g' > "$<.pkg/Resources/Localizable.strings"
	cd "$<.pkg/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../Localizable.strings" "$${i}/Localizable.strings" ; done'
	sh -e "etc/convertscripttojsstring.sh" "$(INSTALL_HOME)/InstallationCheck.langpack" > "$(INSTALL_HOME)/InstallationCheck.js.langpack"
	sh -e "etc/convertscripttojsstring.sh" "$(INSTALL_HOME)/VolumeCheck.langpack" > "$(INSTALL_HOME)/VolumeCheck.js.langpack"
	sed 's#$$(PRODUCT_NAME_AND_VERSION)#$(PRODUCT_NAME) $(PRODUCT_VERSION) $(PRODUCT_LANG_PACK_VERSION)#g' "etc/Distribution.js" | sed 's#$$(PRODUCT_MIN_OSVERSION)#$(PRODUCT_MIN_OSVERSION)#g' | sed 's#$$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#$(INSTALLATION_CHECK_REQUIRED_COMMANDS)#g' | sh -e -c 'sed "s#var.*installationCheckBashScript.*=.*;#var installationCheckBashScript = unescape(\""`cat "$(INSTALL_HOME)/InstallationCheck.js.langpack"`"\");#" | sed "s#var.*volumeCheckBashScript.*=.*;#var volumeCheckBashScript = unescape(\""`cat "$(INSTALL_HOME)/VolumeCheck.js.langpack"`"\");#"' >> "$<.pkg/Distribution"
	cat "$(INSTALL_HOME)/installutils.langpack" "bin/preflight.langpack" > "$<.pkg/contents.pkg/Scripts/preflight" ; chmod a+x "$<.pkg/contents.pkg/Scripts/preflight"
	cat "$(INSTALL_HOME)/installutils.langpack" "bin/postflight.langpack" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION_EXT)#$(PRODUCT_VERSION_EXT)#g' | sed 's#$$(CERTSANDBOXTEAMIDENTIFIER)#$(CERTSANDBOXTEAMIDENTIFIER)#g' > "$<.pkg/contents.pkg/Scripts/postflight" ; chmod a+x "$<.pkg/contents.pkg/Scripts/postflight"
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

build.cd_package: build.package build.source_zip
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	"$(MAKE)" $(MFLAGS) "build.cd_package_shared"
	touch "$@"

build.cd_package_shared:
	sh -e -c 'if [ -d "$(CD_INSTALL_HOME)" ] ; then chmod -Rf a+rw "$(CD_INSTALL_HOME)" ; fi'
	rm -Rf "$(CD_INSTALL_HOME)"
	mkdir -p "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/Language Packs"
	cd "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/Language Packs" ; sh -e -c 'for i in `cd "$(PWD)/$(INSTALL_HOME)" ; find . -type d -name "$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)_*" -maxdepth 1` ; do ( cd "$(PWD)/$(INSTALL_HOME)" ; tar cf - "$$i" ) | tar xf - ; done'
	cd "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" ; ( cd "$(PWD)/$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)" ; tar cf - . ) | tar xf -
	chmod -Rf a-w,a+r "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	chmod -f u+w "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	mv "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
	sync ; hdiutil create -srcfolder "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)" -format UDTO -ov -o "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).cdr.dmg"
	mv "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).cdr.dmg.cdr" "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).cdr.dmg"

build.all: build.package build.package2
	touch "$@"

build.all_patches: build.patch_package build.patch_package2
	touch "$@"
