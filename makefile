#########################################################################
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
CC=cc
CXX=c++
EXTRA_PATH=/opt/local/bin
GNUCP=$(EXTRA_PATH)/gcp
LIBIDL_CONFIG=$(EXTRA_PATH)/libIDL-config-2
PKG_CONFIG=$(EXTRA_PATH)/pkg-config
PKG_CONFIG_PATH=$(EXTRA_PATH)/../lib/pkgconfig/
PRODUCT_NAME=My Untested Office Suite
PRODUCT_DIR_NAME=My_Untested_Office_Suite
PRODUCT_TRADEMARKED_NAME=$(PRODUCT_NAME)
PRODUCT_TRADEMARKED_NAME_RTF=$(PRODUCT_NAME)

# Custom overrides go in the following file
-include custom.mk

# Set the shell to tcsh since the OpenOffice.org build requires it
ifndef TMP
TMP:=/tmp
endif
SHELL:=/bin/tcsh
UNAME:=$(shell uname -p)
OS_MAJOR_VERSION:=$(shell /usr/bin/sw_vers | grep '^ProductVersion:' | awk '{ print $$2 }' | awk -F. '{ print $$1 "." $$2 }')
ifeq ("$(UNAME)","powerpc")
ULONGNAME=PowerPC
CPUNAME=P
UOUTPUTDIR=unxmacxp.pro
DLLSUFFIX=mxp
TARGET_FILE_TYPE=Mach-O executable ppc
else
ULONGNAME=Intel
CPUNAME=I
UOUTPUTDIR=unxmacxi.pro
DLLSUFFIX=mxi
TARGET_FILE_TYPE=Mach-O executable i386
endif
COMPILERDIR=$(BUILD_HOME)/solenv/`basename $(UOUTPUTDIR) .pro`/bin

# Build location macros
BUILD_HOME:=build
INSTALL_HOME:=install
PATCH_INSTALL_HOME:=patch_install
SOURCE_HOME:=source
CD_INSTALL_HOME:=cd_install
OO_PATCHES_HOME:=patches/openoffice
OOO-BUILD_PATCHES_HOME:=patches/ooo-build
ODF-CONVERTER_PATCHES_HOME:=patches/odf-converter
IMEDIA_PATCHES_HOME:=patches/imedia
REMOTECONTROL_PATCHES_HOME:=patches/remotecontrol
ifeq ("$(UNAME)","powerpc")
OO_ENV_AQUA:=$(BUILD_HOME)/MacOSXPPCEnv.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacOSXPPCEnvJava.Set
else
OO_ENV_AQUA:=$(BUILD_HOME)/MacOSXX86Env.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacOSXX86EnvJava.Set
endif
OO_LANGUAGES=ALL
NEOLIGHT_MDIMPORTER_URL:=http://trinity.neooffice.org/downloads/neolight.mdimporter.tgz
NEOLIGHT_MDIMPORTER_ID:=org.neooffice.neolight
NEOPEEK_QLPLUGIN_URL:=http://trinity.neooffice.org/downloads/neopeek.qlgenerator.tgz
NEOPEEK_QLPLUGIN_ID:=org.neooffice.quicklookplugin

# Product information
OO_PRODUCT_NAME=OpenOffice.org
OO_PRODUCT_VERSION=3.0.1
OO_REGISTRATION_URL=http://survey.services.openoffice.org/user/index.php
PRODUCT_VERSION_FAMILY=3.0
PRODUCT_VERSION=3.0.1
PRODUCT_DIR_VERSION=3.0.1
PREVIOUS_PRODUCT_VERSION=$(PRODUCT_VERSION)
PRODUCT_LANG_PACK_VERSION=Language Pack
PRODUCT_DIR_LANG_PACK_VERSION=Language_Pack
PRODUCT_PATCH_VERSION=Patch 2
PRODUCT_DIR_PATCH_VERSION=Patch-2
PRODUCT_BASE_URL=http://www.neooffice.org/neojava
PRODUCT_REGISTRATION_URL=http://trinity.neooffice.org/modules.php?name=Your_Account\&amp\;redirect=index
PRODUCT_SUPPORT_URL=http://www.neooffice.org/neojava/contact.php
PRODUCT_SUPPORT_URL_TEXT:=$(PRODUCT_NAME) Support
ifneq ($(findstring Early Access,$(PRODUCT_VERSION)),)
PRODUCT_DOWNLOAD_URL=http://www.neooffice.org/neojava/earlyaccessdownload.php?fragment=download
else
PRODUCT_DOWNLOAD_URL=http://www.neooffice.org/neojava/download.php?fragment=download
endif
PRODUCT_DOWNLOAD_URL_TEXT=$(PRODUCT_NAME) Downloads
ifneq ($(findstring Early Access,$(PRODUCT_VERSION)),)
PRODUCT_DOWNLOADLANGPACK_URL=http://www.neooffice.org/neojava/earlyaccesslangpackdownload.php
else
PRODUCT_DOWNLOADLANGPACK_URL=http://www.neooffice.org/neojava/langpackdownload.php
endif
PRODUCT_DOCUMENTATION_URL=http://neowiki.neooffice.org/
PRODUCT_DOCUMENTATION_URL_TEXT=$(PRODUCT_NAME) Wiki
PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL=http://neowiki.neooffice.org/index.php/NeoOffice_Launch_Shortcuts
PRODUCT_DOCUMENTATION_SPELLCHECK_URL=http://neowiki.neooffice.org/index.php/Activating_Dictionaries_and_Configuring_Spellcheck
PRODUCT_UPDATE_CHECK_URL=$(PRODUCT_BASE_URL)/patchcheck.php
PRODUCT_COMPONENT_MODULES=grammarcheck imagecapture mediabrowser neomobile remotecontrol
PRODUCT_COMPONENT_PATCH_MODULES=neomobile

# CVS macros
OO_CVSROOT:=:pserver:anoncvs@anoncvs.services.openoffice.org:/cvs
OO_PACKAGES:=OpenOffice3 Extensions3
OO_TAG:=-rOOO300_m14
OOO-BUILD_SVNROOT:=http://svn.gnome.org/svn/ooo-build/branches/ooo-build-3-0-1
OOO-BUILD_PACKAGE:=ooo-build
OOO-BUILD_TAG:=--revision '{2008-12-30}'
OOO-BUILD_APPLY_TAG:=OOO300_m14
LPSOLVE_SOURCE_URL=http://download.go-oo.org/SRC680/lp_solve_5.5.tar.gz
LIBWPD_SOURCE_URL=http://download.go-oo.org/libwpd/libwpd-0.8.14.tar.gz
LIBWPG_SOURCE_URL=http://download.go-oo.org/SRC680/libwpg-0.1.3.tar.gz
LIBWPS_SOURCE_URL=http://download.go-oo.org/SRC680/libwps-0.1.2.tar.gz
MOZ_SOURCE_URL=ftp://ftp.mozilla.org/pub/mozilla.org/mozilla/releases/mozilla1.7.5/source/mozilla-source-1.7.5.tar.gz
ODF-CONVERTER_PACKAGE=odf-converter-2.5
IMEDIA_SVNROOT=http://imedia.googlecode.com/svn/branches/1.x/
IMEDIA_PACKAGE=imedia-read-only
IMEDIA_TAG:=--revision '{2008-12-11}'
REMOTECONTROL_PACKAGE=remotecontrol
REMOTECONTROL_ZIP_URL=http://martinkahr.com/files/source/RemoteControlWrapper_R962.tgz
REMOTECONTROL_ZIP_FILENAME=RemoteControlWrapper_R962.tgz
NEO_CVSROOT:=:pserver:anoncvs@anoncvs.neooffice.org:/cvs
NEO_PACKAGE:=NeoOffice
NEO_TAG:=-rNeoOffice-3_0_1

all: build.all

# Include dependent makefiles
include neo_configure.mk

# Create the build directory and checkout the OpenOffice.org sources
build.oo_checkout:
	mkdir -p "$(BUILD_HOME)"
# The OOo cvs server gets messed up with tags so we need to do a little trick
# to get the checkout to work
	rm -Rf "$(BUILD_HOME)/tmp" ; mkdir -p "$(BUILD_HOME)/tmp" ; cd "$(BUILD_HOME)/tmp" ; cvs -d "$(OO_CVSROOT)" co MathMLDTD ; cd MathMLDTD ; cvs update -d $(OO_TAG)
	rm -Rf "$(BUILD_HOME)/tmp"
# Do the real checkout
	cd "$(BUILD_HOME)" ; cvs -d "$(OO_CVSROOT)" co $(OO_TAG) $(OO_PACKAGES)
# cvs seems to always fail so check that the last module has been checked out
	cd "$(BUILD_HOME)/basebmp" ; cvs -d "$(OO_CVSROOT)" update $(OO_TAG)
	chmod -Rf u+w "$(BUILD_HOME)"
	touch "$@"

build.ooo-build_checkout: build.oo_checkout
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; svn co $(OOO-BUILD_TAG) $(OOO-BUILD_SVNROOT) "$(OOO-BUILD_PACKAGE)"
	cd "$(BUILD_HOME)" ; chmod -Rf u+w "$(OOO-BUILD_PACKAGE)"
	touch "$@"

build.odf-converter_checkout: $(ODF-CONVERTER_PATCHES_HOME)/$(ODF-CONVERTER_PACKAGE).tar.gz
	rm -Rf "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)"
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; tar zxvf "$(PWD)/$<"
	cd "$(BUILD_HOME)" ; chmod -Rf u+rw "$(ODF-CONVERTER_PACKAGE)"
	touch "$@"

build.imedia_checkout:
	rm -Rf "$(BUILD_HOME)/$(IMEDIA_PACKAGE)"
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; svn co $(IMEDIA_TAG) $(IMEDIA_SVNROOT) "$(IMEDIA_PACKAGE)"
	cd "$(BUILD_HOME)" ; chmod -Rf u+w "$(IMEDIA_PACKAGE)"
	touch "$@"

build.remotecontrol_checkout:
	rm -Rf "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)"
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; mkdir "$(REMOTECONTROL_PACKAGE)"
	cd "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)" ; curl -L -O "$(REMOTECONTROL_ZIP_URL)"
	cd "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)" ; tar xvfz "$(REMOTECONTROL_ZIP_FILENAME)"
	touch "$@"

build.oo_patches: \
	build.oo_config_office_patch \
	build.oo_cppu_patch \
	build.oo_cppuhelper_patch \
	build.oo_cpputools_patch \
	build.oo_external_patch \
	build.oo_filter_patch \
	build.oo_framework_patch \
	build.oo_i18npool_patch \
	build.oo_jvmfwk_patch \
	build.oo_lingucomponent_patch \
	build.oo_moz_patch \
	build.oo_solenv_patch \
	build.oo_sw_patch \
	build.oo_testshl2_patch \
	build.oo_vcl_patch \
	build.oo_vos_patch
	touch "$@"

build.oo_310_patches: \
	build.oo_310_comphelper_patch \
	build.oo_310_dbaccess_patch \
	build.oo_310_filter_patch \
	build.oo_310_offapi_patch \
	build.oo_310_officecfg_patch \
	build.oo_310_sfx2_patch \
	build.oo_310_svtools_patch \
	build.oo_310_svx_patch \
	build.oo_310_uui_patch
	touch "$@"

build.oo_odk_patches: build.oo_patches
	touch "$@"

build.oo_external_patch: build.ooo-build_patches \
	$(OO_PATCHES_HOME)/gpc231.tar.Z
	chmod -Rf u+w "$(BUILD_HOME)/external/gpc"
	gnutar zxf "$(OO_PATCHES_HOME)/gpc231.tar.Z" -C "$(BUILD_HOME)/external/gpc"
	chmod -Rf u+w "$(BUILD_HOME)/external/gpc"
	mv -f "$(BUILD_HOME)/external/gpc/gpc231"/* "$(BUILD_HOME)/external/gpc"
	rm -Rf "$(BUILD_HOME)/external/gpc/gpc231"
	touch "$@"

build.oo_moz_patch: $(OO_PATCHES_HOME)/moz.patch build.ooo-build_patches
	-( cd "$(BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	cd "$(BUILD_HOME)/moz/download" ; curl -L -O "$(MOZ_SOURCE_URL)"
	touch "$@"

build.oo_310_%_patch: $(OO_PATCHES_HOME)/3.1.0/%.diff build.oo_patches
	-( cd "$(BUILD_HOME)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.oo_%_patch: $(OO_PATCHES_HOME)/%.patch build.ooo-build_patches
	-( cd "$(BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.ooo-build_patches: build.ooo-build_checkout \
	build.ooo-build_apply_patch
	touch "$@"

build.ooo-build_apply_patch: $(OOO-BUILD_PATCHES_HOME)/apply.patch build.oo_checkout build.ooo-build_checkout
	-( cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	rm -f "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)/patches/apply.pl" ; sed 's#@GNUPATCH@#patch#g' "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)/patches/apply.pl.in" > "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)/patches/apply.pl" ; chmod a+x "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)/patches/apply.pl" ; "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)/patches/apply.pl" --tag="$(OOO-BUILD_APPLY_TAG)" --distro=MacOSX "$(PWD)/$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)/patches/dev300" "$(PWD)/$(BUILD_HOME)"
	cp "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)/src/go-oo-team.png" "$(BUILD_HOME)/default_images/sw/res"
	cp "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)/src/evolocal.odb" "$(BUILD_HOME)/extras/source/database"
	mkdir -p "$(BUILD_HOME)/lpsolve/download" ; cd "$(BUILD_HOME)/lpsolve/download" ; curl -L -O "$(LPSOLVE_SOURCE_URL)"
	mkdir -p "$(BUILD_HOME)/libwpd/download" ; cd "$(BUILD_HOME)/libwpd/download" ; curl -L -O "$(LIBWPD_SOURCE_URL)"
	mkdir -p "$(BUILD_HOME)/libwpg/download" ; cd "$(BUILD_HOME)/libwpg/download" ; curl -L -O "$(LIBWPG_SOURCE_URL)"
	mkdir -p "$(BUILD_HOME)/libwps/download" ; cd "$(BUILD_HOME)/libwps/download" ; curl -L -O "$(LIBWPS_SOURCE_URL)"
	touch "$@"

build.ooo-build_%_patch: $(OOO-BUILD_PATCHES_HOME)/%.patch build.oo_checkout build.ooo-build_checkout
	-( cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.odf-converter_patches: $(ODF-CONVERTER_PATCHES_HOME)/odf-converter.patch build.odf-converter_checkout
	-( cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<" 
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)" ; setenv PATH "$(PWD)/$(COMPILERDIR)":/usr/bin:"$$PATH" ; "$(MAKE)" $(MFLAGS)
	rm -Rf "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist"
	mkdir -p "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist"
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; ( ( cd "`/usr/bin/pkg-config --variable=prefix mono`/etc" ; gnutar cvf - mono ) | ( cd "$(PWD)/$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; gnutar xvf - ) )
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; cp "$(PWD)/$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/source/Shell/OdfConverter/OdfConverter" "OdfConverter" ; chmod a+x "OdfConverter" ; otool -L "OdfConverter" | awk '{ print $$1 }' | grep '\.dylib' | grep -v ':$$' | grep -v '^\/usr\/lib\/' | grep -v '^\/System\/Library\/Frameworks\/' | sed 's#^@executable_path/\.\./lib/#/Library/Frameworks/Mono.framework/Libraries/#' > "library.list"
# Find all non-system linked libraries
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; touch "library.list.bak" ; sh -c -e 'while ! diff -q "library.list" "library.list.bak" >/dev/null ; do cp "library.list" "library.list.bak" ; for i in `cat "library.list"` ; do otool -L "$$i" | awk "{ print \$$1 }" | grep "\.dylib" | grep -v ":\$$" | grep -v "^\/usr\/lib\/" | grep -v "^\/System\/Library\/Frameworks\/" | sed "s#^@executable_path/\.\./lib/#/Library/Frameworks/Mono.framework/Libraries/#" >> "library.list" ; done ; sort -u "library.list" > "library.list.tmp" ; mv "library.list.tmp" "library.list" ; done' ; rm "library.list.bak"
# Resolve and copy all softlinks
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; touch "library.list.bak" ; sh -c -e 'while ! diff -q "library.list" "library.list.bak" >/dev/null ; do cp "library.list" "library.list.bak" ; for i in `cat "library.list"` ; do if [ -h "$$i" ] ; then linkedfile=`ls -l "$$i" | awk "{ print \\$$NF }"` ; dirname=`dirname "$$i"` ; if [ -f "$$dirname/$$linkedfile" ] ; then echo "$$dirname/$$linkedfile" >> "library.list" ; ln -sf "$$linkedfile" `basename "$$i"` ; fi ; fi ; done ; sort -u "library.list" > "library.list.tmp" ; mv "library.list.tmp" "library.list" ; done' ; rm "library.list.bak"
# Copy all files
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; sh -c -e 'for i in `cat "library.list"` ; do if [ -f "$$i" ] ; then cp "$$i" `basename "$$i"` ; fi ; done'
# Change each library's internal name to @executable_path/libname
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; sh -c -e 'for i in OdfConverter `cat "library.list"` ; do basename=`basename "$$i"` ; install_name_tool -id "@executable_path/$$basename" "$$basename" ; done'
# Change each library's link list to @executable_path/linkedlibname
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; sh -c -e 'for i in OdfConverter `cat "library.list"` ; do for j in `cat "library.list"` ; do basename=`basename "$$i"` ; install_name_tool -change "$$j" @executable_path/`basename "$$j"` "$$basename" ; install_name_tool -change @executable_path/../lib/`basename "$$j"` @executable_path/`basename "$$j"` "$$basename" ; done ; done'
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; rm "library.list"
	touch "$@"

build.imedia_src_untar: $(IMEDIA_PATCHES_HOME)/additional_source build.imedia_checkout
	cd "$(BUILD_HOME)/$(IMEDIA_PACKAGE)" ; ( cd "$(PWD)/$<" ; tar cf - *.h *.m *.png *.lproj ) | tar xvf -
	touch "$@"

build.imedia_patches: $(IMEDIA_PATCHES_HOME)/imedia.patch build.imedia_src_untar
	-( cd "$(BUILD_HOME)/$(IMEDIA_PACKAGE)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(IMEDIA_PACKAGE)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	cd "$(BUILD_HOME)/$(IMEDIA_PACKAGE)" ; xcodebuild -target iMediaBrowser -configuration Debug clean
	cd "$(BUILD_HOME)/$(IMEDIA_PACKAGE)" ; xcodebuild -target iMediaBrowser -configuration Debug
	touch "$@"

build.remotecontrol_patches: $(REMOTECONTROL_PATCHES_HOME)/additional_source build.remotecontrol_checkout
	cd "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)" ; ( cd "$(PWD)/$<" ; tar cf - *.xcodeproj *.plist ) | tar xvf -
	cd "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)" ; xcodebuild -project RemoteControlFramework.xcodeproj -target RemoteControl -configuration Release clean
	cd "$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)" ; xcodebuild -project RemoteControlFramework.xcodeproj -target RemoteControl -configuration Release
	touch "$@"
	
build.configure: build.oo_310_patches
	cd "$(BUILD_HOME)/config_office" ; autoconf
	( cd "$(BUILD_HOME)/config_office" ; setenv PATH "$(PWD)/$(COMPILERDIR):/bin:/sbin:/usr/bin:/usr/sbin:$(EXTRA_PATH)" ; unsetenv DYLD_LIBRARY_PATH ; ./configure CC=$(CC) CXX=$(CXX) PKG_CONFIG=$(PKG_CONFIG) PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) TMP=$(TMP) --with-jdk-home=/System/Library/Frameworks/JavaVM.framework/Home --with-java-target-version=1.4 --with-epm=internal --enable-vba --disable-cups --disable-gtk --disable-odk --without-nas --with-mozilla-toolkit=xlib --with-gnu-cp="$(GNUCP)" --with-system-curl --with-lang="$(OO_LANGUAGES)" --disable-headless --disable-pasf --disable-fontconfig --disable-binfilter --without-system-mdbtools --enable-minimizer --enable-presenter-console --enable-pdfimport --enable-wiki-publisher --enable-ogltrans --enable-report-builder )
	echo 'setenv LIBIDL_CONFIG "$(LIBIDL_CONFIG)"' >> "$(OO_ENV_AQUA)"
	echo 'setenv PKG_CONFIG "$(PKG_CONFIG)"' >> "$(OO_ENV_AQUA)"
	echo 'setenv PKG_CONFIG_PATH "$(PKG_CONFIG_PATH)"' >> "$(OO_ENV_AQUA)"
	echo 'setenv TMP "$(TMP)"' >> "$(OO_ENV_AQUA)"
	echo 'setenv TEMP "$(TMP)"' >> "$(OO_ENV_AQUA)"
	( cd "$(BUILD_HOME)" ; ./bootstrap )
	touch "$@"

build.oo_all: build.configure
	source "$(OO_ENV_AQUA)" ; cd "$(BUILD_HOME)/instsetoo_native" ; `alias build` --all $(OO_BUILD_ARGS)
	touch "$@"

build.neo_configure: build.oo_all neo_configure.mk
	rm -f "$(OO_ENV_JAVA)"
	$(MAKE) $(MFLAGS) build.neo_configure_phony
	touch "$@"

build.neo_%_patch: % build.neo_configure
	cd "$<" ; sh -e -c 'for i in `cd "$(PWD)/$(BUILD_HOME)/$<" ; find . -type d | grep -v /CVS$$ | grep -v /$(UOUTPUTDIR) | grep -v /quicktime` ; do mkdir -p "$$i" ; done'
	cd "$<" ; sh -e -c 'for i in `cd "$(PWD)/$(BUILD_HOME)/$<" ; find . ! -type d | grep -v /CVS/ | grep -v /$(UOUTPUTDIR) | grep -v /quicktime` ; do if [ ! -f "$$i" ] ; then ln -sf "$(PWD)/$(BUILD_HOME)/$</$$i" "$$i" 2>/dev/null ; fi ; done'
	sh -e -c 'if [ ! -d "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR).oo" -a -d "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR)" ] ; then rm -Rf "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR).oo" ; mv -f "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR)" "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR).oo" ; fi'
	rm -Rf "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR)"
	mkdir -p "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR)"
	cd "$<" ; ln -sf "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR)"
	source "$(OO_ENV_JAVA)" ; cd "$<" ; `alias build` $(NEO_BUILD_ARGS)
	touch "$@"

build.neo_%_component: % build.neo_configure
	rm -Rf "$(PWD)/$</$(UOUTPUTDIR)"
	mkdir -p "$(PWD)/$</$(UOUTPUTDIR)"
	source "$(OO_ENV_JAVA)" ; cd "$<" ; `alias build` $(NEO_BUILD_ARGS)
	touch "$@"

build.neo_patches: build.oo_all \
	build.imedia_patches \
	build.remotecontrol_patches \
	build.odf-converter_patches \
	$(PRODUCT_COMPONENT_MODULES:%=build.neo_%_component) \
	$(PRODUCT_COMPONENT_PATCH_MODULES:%=build.neo_%_component) \
	build.neo_avmedia_patch \
	build.neo_basic_patch \
	build.neo_binfilter_patch \
	build.neo_bridges_patch \
	build.neo_canvas_patch \
	build.neo_configmgr_patch \
	build.neo_connectivity_patch \
	build.neo_cppcanvas_patch \
	build.neo_cppuhelper_patch \
	build.neo_cpputools_patch \
	build.neo_desktop_patch \
	build.neo_extensions_patch \
	build.neo_filter_patch \
	build.neo_fpicker_patch \
	build.neo_framework_patch \
	build.neo_goodies_patch \
	build.neo_hsqldb_patch \
	build.neo_jvmfwk_patch \
	build.neo_lingucomponent_patch \
	build.neo_package_patch \
	build.neo_rhino_patch \
	build.neo_reportdesign_patch \
	build.neo_sal_patch \
	build.neo_sc_patch \
	build.neo_sd_patch \
	build.neo_sdext_patch \
	build.neo_shell_patch build.neo_sfx2_patch \
	build.neo_store_patch \
	build.neo_svtools_patch \
	build.neo_svx_patch \
	build.neo_sw_patch \
	build.neo_vcl_patch \
	build.neo_dtrans_patch \
	build.neo_ucb_patch \
	build.neo_ucbhelper_patch \
	build.neo_libwpd_patch build.neo_writerperfect_patch \
	build.neo_xmloff_patch
	touch "$@"

build.neo_odk_patches: \
	build.oo_odk_all \
	build.neo_odk_patch
	touch "$@"

build.package: build.neo_patches
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	"$(MAKE)" $(MFLAGS) "build.package_shared"
	touch "$@"

build.package_shared:
	sh -e -c 'if [ -d "$(INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(INSTALL_HOME)" ; fi'
	sh -e -c 'if [ -d "/Volumes/OpenOffice.org $(PRODUCT_VERSION_FAMILY)" ] ; then hdiutil unmount "/Volumes/OpenOffice.org $(PRODUCT_VERSION_FAMILY)" ; fi'
	hdiutil mount "$(BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice/dmg/install/en-US/OOo_$(OO_PRODUCT_VERSION)_"*"$(ULONGNAME)_install.dmg"
	mkdir -p "$(INSTALL_HOME)/package/Contents"
	cd "$(INSTALL_HOME)/package" ; ( ( cd "/Volumes/OpenOffice.org $(PRODUCT_VERSION_FAMILY)/OpenOffice.org.app" ; gnutar cvf - . ) | ( cd "$(PWD)/$(INSTALL_HOME)/package" ; gnutar xvf - --exclude="._*" ) )
	hdiutil unmount "/Volumes/OpenOffice.org $(PRODUCT_VERSION_FAMILY)"
# Remove OOo system plugins but fix bug 3381 to save standard dictionaries
	rm -Rf "$(INSTALL_HOME)/package/Contents/Frameworks"
	rm -Rf "$(INSTALL_HOME)/package/Contents/Library"
	rm -Rf "$(INSTALL_HOME)/package/Contents/share/extension"
	mkdir -p "$(INSTALL_HOME)/package/Contents/share/extension"
# Regroup the OOo language packs
	cd "$(BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/install" ; find . -type d -maxdepth 1 -exec basename {} \; | grep -v '^\.$$' | grep -v '^log$$' > "$(PWD)/$(INSTALL_HOME)/language_names"
ifdef NOLANGPACKS
# Bypass the language pack installers
else
# Create the language pack installers
	sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names"` ; do if [ "$${i}" = "en-US" ] ; then continue ; fi ; langname=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | sed "s/#.*$$//" | awk -F, "{ print \\$$3 }"` ; langdirname=`echo "$${langname}" | sed "s# #_#g"` ; if [ -z "$${langname}" -o -z "$${langdirname}" ] ; then echo "Skipping $${i} language..." ; continue ; fi ; mkdir -p "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents" ; if [ -d "/Volumes/OpenOffice.org $(PRODUCT_VERSION_FAMILY)" ] ; then hdiutil unmount "/Volumes/OpenOffice.org Languagepack" ; fi ; hdiutil mount "$(PWD)/$(BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/install/$${i}/OpenOffice.org-langpack-$(OO_PRODUCT_VERSION)_$${i}.dmg" ; bunzip2 -dc "/Volumes/OpenOffice.org Languagepack/OpenOffice.org Languagepack.app/Contents/tarball.tar.bz2" | ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}" ; gnutar xvf - --exclude="._*" ) ; hdiutil unmount "/Volumes/OpenOffice.org Languagepack" ; rm -f "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/MacOS/resource/ooo"*.res ; cp "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/bin/ooo$${i}.res" "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/bin/salapp$${i}.res" "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/MacOS/resource" ; helpflag=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | awk -F, "{ print \\$$2 }"` ; if [ "$${helpflag}" != "1" ] ; then rm -Rf "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help/$${i}" ; ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help" ; ln -s "en" "$${i}" ) ; fi ; "$(MAKE)" $(MFLAGS) "PRODUCT_LANG_PACK_LOCALE=$${i}" "PRODUCT_LANG_PACK_VERSION=$(PRODUCT_LANG_PACK_VERSION) $${langname}" "PRODUCT_DIR_LANG_PACK_VERSION=$(PRODUCT_DIR_LANG_PACK_VERSION)_$${langdirname}" "build.package_$${langdirname}" ; done'
endif
	chmod -Rf u+w,a+r "$(INSTALL_HOME)/package"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/avmedia/$(UOUTPUTDIR)/lib/libavmedia$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/avmedia/$(UOUTPUTDIR)/lib/libavmediaquicktime.dylib" "$(PWD)/$(BUILD_HOME)/basic/$(UOUTPUTDIR)/lib/libsb$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/canvas/$(UOUTPUTDIR)/lib/vclcanvas.uno.dylib" "$(PWD)/$(BUILD_HOME)/configmgr/$(UOUTPUTDIR)/lib/configmgr2.uno.dylib" "$(PWD)/$(BUILD_HOME)/connectivity/$(UOUTPUTDIR)/lib/libmacab1.dylib" "$(PWD)/$(BUILD_HOME)/connectivity/$(UOUTPUTDIR)/lib/libmacabdrv1.dylib" "$(PWD)/$(BUILD_HOME)/connectivity/$(UOUTPUTDIR)/lib/libmozab2.dylib" "$(PWD)/$(BUILD_HOME)/connectivity/$(UOUTPUTDIR)/lib/libmozabdrv2.dylib" "$(PWD)/$(BUILD_HOME)/connectivity/$(UOUTPUTDIR)/lib/libodbcbase2.dylib" "$(PWD)/$(BUILD_HOME)/cppcanvas/$(UOUTPUTDIR)/lib/libcppcanvas$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/deployment$(DLLSUFFIX).uno.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/deploymentgui$(DLLSUFFIX).uno.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/libdeploymentmisc$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/libsofficeapp.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/libspl$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/libunopkgapp.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/migrationoo2.uno.dylib" "$(PWD)/$(BUILD_HOME)/dtrans/$(UOUTPUTDIR)/lib/libdtransjava$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/extensions/$(UOUTPUTDIR)/lib/libscn$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/extensions/$(UOUTPUTDIR)/lib/libupdchk$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/extensions/$(UOUTPUTDIR)/lib/updchk.uno.dylib" "$(PWD)/$(BUILD_HOME)/filter/$(UOUTPUTDIR)/lib/libpdffilter$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/fpicker/$(UOUTPUTDIR)/lib/fpicker.uno.dylib" "$(PWD)/$(BUILD_HOME)/fpicker/$(UOUTPUTDIR)/lib/fps_java.uno.dylib" "$(PWD)/$(BUILD_HOME)/framework/$(UOUTPUTDIR)/lib/libfwe$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/framework/$(UOUTPUTDIR)/lib/libfwk$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/goodies/$(UOUTPUTDIR)/lib/libgo$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/goodies/$(UOUTPUTDIR)/lib/libipt$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/lingucomponent/$(UOUTPUTDIR)/lib/libspell$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/package/$(UOUTPUTDIR)/lib/libxstor.dylib" "$(PWD)/$(BUILD_HOME)/reportdesign/$(UOUTPUTDIR)/lib/librpt$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/sc/$(UOUTPUTDIR)/lib/libsc$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/sd/$(UOUTPUTDIR)/lib/libsdui$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/sfx2/$(UOUTPUTDIR)/lib/libsfx$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/shell/$(UOUTPUTDIR)/lib/cmdmail.uno.dylib" "$(PWD)/$(BUILD_HOME)/shell/$(UOUTPUTDIR)/lib/localebe1.uno.dylib" "$(PWD)/$(BUILD_HOME)/shell/$(UOUTPUTDIR)/lib/macbe1.uno.dylib" "$(PWD)/$(BUILD_HOME)/shell/$(UOUTPUTDIR)/lib/syssh.uno.dylib" "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/lib/libcui$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/svtools/$(UOUTPUTDIR)/lib/libsvl$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/svtools/$(UOUTPUTDIR)/lib/libsvt$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/lib/libsvx$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/sw/$(UOUTPUTDIR)/lib/libsw$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/ucb/$(UOUTPUTDIR)/lib/libucpdav1.dylib" "$(PWD)/$(BUILD_HOME)/ucbhelper/$(UOUTPUTDIR)/lib/libucbhelper4gcc3.dylib" "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/lib/libvcl$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/lib/libvcljava1.dylib" "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/lib/libvcljava2.dylib" "$(PWD)/$(BUILD_HOME)/writerperfect/$(UOUTPUTDIR)/lib/libmsworks$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/writerperfect/$(UOUTPUTDIR)/lib/libwpft$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/writerperfect/$(UOUTPUTDIR)/lib/libwpgimport$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/xmloff/$(UOUTPUTDIR)/lib/libxo$(DLLSUFFIX).dylib" "basis-link/program"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/bridges/$(UOUTPUTDIR)/lib/libgcc3_uno.dylib" "$(PWD)/$(BUILD_HOME)/cppuhelper/$(UOUTPUTDIR)/lib/libuno_cppuhelpergcc3.dylib.3" "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/lib/libjvmfwk.dylib.3" "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/lib/sunjavaplugin.dylib" "$(PWD)/$(BUILD_HOME)/sal/$(UOUTPUTDIR)/lib/libuno_sal.dylib.3" "$(PWD)/$(BUILD_HOME)/store/$(UOUTPUTDIR)/lib/libstore.dylib.3" "basis-link/ure-link/lib"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/bin/salappen-US.res" "MacOS/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/cpputools/$(UOUTPUTDIR)/bin/uno" "basis-link/ure-link/bin/uno.bin" ; chmod a+x "basis-link/ure-link/bin/uno.bin"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/misc/soffice.sh" "MacOS/soffice" ; chmod a+x "MacOS/soffice"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/bin/soffice" "MacOS/soffice.bin" ; chmod a+x "MacOS/soffice.bin"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/bin/unopkg" "MacOS/unopkg.bin" ; chmod a+x "MacOS/unopkg.bin"
	cd "$(INSTALL_HOME)/package/Contents" ; rm -f "MacOS/resource/ooo"*.res ; cp -f "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/bin/oooen-US.res" "MacOS/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/shell/$(UOUTPUTDIR)/bin/senddoc" "basis-link/program/senddoc" ; chmod a+x "basis-link/program/senddoc"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/bin/javavendors.xml" "basis-link/ure-link/share/misc/javavendors.xml"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/extensions/$(UOUTPUTDIR)/misc/registry/spool/org/openoffice/Office/Addons/Addons-onlineupdate.xcu" "basis-link/share/registry/data/org/openoffice/Office/Addons.xcu"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/extensions/$(UOUTPUTDIR)/misc/registry/spool/org/openoffice/Office/Jobs/Jobs-onlineupdate.xcu" "basis-link/share/registry/data/org/openoffice/Office/Jobs.xcu"
# Fix bug 3426 by including the legacy binfilter files
	cd "$(INSTALL_HOME)/package/Contents" ; sh -c -e 'for i in `cd "$(PWD)/$(BUILD_HOME)/binfilter/$(UOUTPUTDIR)/lib" ; find . -name "*.dylib*" -maxdepth 1` ; do cp "$(PWD)/$(BUILD_HOME)/binfilter/$(UOUTPUTDIR)/lib/$${i}" "basis-link/program/$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/binfilter/$(UOUTPUTDIR)/bin/legacy_binfilters.rdb" "basis-link/program/legacy_binfilters.rdb"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -c -e 'for i in `cd "$(PWD)/$(BUILD_HOME)/binfilter/$(UOUTPUTDIR)/bin" ; find . -name "*en-US.res" -maxdepth 1` ; do cp "$(PWD)/$(BUILD_HOME)/binfilter/$(UOUTPUTDIR)/bin/$${i}" "basis-link/program/resource/$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/filter/$(UOUTPUTDIR)/misc/filters/ui/langpacks/en-US/org/openoffice/TypeDetection/Filter.xcu" "basis-link/share/registry/res/en-US/org/openoffice/TypeDetection/Filter.xcu"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -c -e 'for i in `cd "$(PWD)/$(BUILD_HOME)/filter/$(UOUTPUTDIR)/misc/filters/modulepacks" ; find . -name "*_bf_filters.xcu" -maxdepth 1` ; do cp "$(PWD)/$(BUILD_HOME)/filter/$(UOUTPUTDIR)/misc/filters/modulepacks/$${i}" "basis-link/share/registry/modules/org/openoffice/TypeDetection/Filter/$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -c -e 'for i in `cd "$(PWD)/$(BUILD_HOME)/filter/$(UOUTPUTDIR)/misc/filters/modulepacks" ; find . -name "*_bf_types.xcu" -maxdepth 1` ; do cp "$(PWD)/$(BUILD_HOME)/filter/$(UOUTPUTDIR)/misc/filters/modulepacks/$${i}" "basis-link/share/registry/modules/org/openoffice/TypeDetection/Types/$${i}" ; done'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libbf_migratefilter$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libbindet$(DLLSUFFIX).dylib'
# Include the uui*.res files as they contain several new GUI resources
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names"` ; do langname=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | sed "s/#.*$$//" | awk -F, "{ print \\$$1 }"` ; if [ -z "$${langname}" ] ; then echo "Skipping $${i} language..." ; continue ; fi ; cp "$(PWD)/$(BUILD_HOME)/uui/$(UOUTPUTDIR)/bin/uui$${langname}.res" "basis-link/program/resource/uui$${langname}.res" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/package/Info.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME)#$(PRODUCT_TRADEMARKED_NAME)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PRODUCT_FILETYPE)#$(PRODUCT_FILETYPE)#g' > "Info.plist"
	cd "$(INSTALL_HOME)/package/Contents" ; printf '%s' 'APPL$(PRODUCT_FILETYPE)' > "PkgInfo"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/hsqldb/$(UOUTPUTDIR)/misc/build/hsqldb/lib/hsqldb.jar" "basis-link/program/classes"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/rhino/$(UOUTPUTDIR)/misc/build/rhino1_5R5/build/rhino1_5R5/js.jar" "basis-link/program/classes"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/class/vcl.jar" "basis-link/program/classes"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#^ProgressPosition=.*$$#ProgressPosition=14,260#g' "program/sofficerc" > "../../out" ; mv -f "../../out" "program/sofficerc"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find "basis-link/share/config/soffice.cfg/modules" -name "menubar.xml"` ; do sed "s#<menu:menuitem.*\.uno:TwainSelect.*/>#<\!--&-->#g" "$${i}" > "../../out" ; mv -f "../../out" "$${i}" ; done'
	rm -Rf "$(INSTALL_HOME)/package/Contents/Resources"
	mkdir -p "$(INSTALL_HOME)/package/Contents/Resources"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/package/ship.icns" "Resources"
# Add additional ooo-build icons
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp/res/commandimagelist"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; cp `find "$(PWD)/$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)/src/icons" -name "*togglesheetgrid.png" -maxdepth 1` "res/commandimagelist"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" .
# Add special ooo-build icons for Tango
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; cp `find "$(PWD)/$(BUILD_HOME)/$(OOO-BUILD_PACKAGE)/src/icons/tango" -name "*togglesheetgrid.png" -maxdepth 1` "res/commandimagelist"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" .
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/neo2toolbarv10.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; find svtools svx -type f > "$(PWD)/$(INSTALL_HOME)/toolbaricons"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "tmp/NeoOffice Toolbar & Preferences Icons 1.0/source/Generic Template.icns" "Resources/generic.icns"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/AkuaIcons.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp/Akua_2009Q4_Release"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp/Akua_2009Q4_Release/images"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp/Akua_2009Q4_Release/images"
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2009Q4_Release/images" ; unzip "$(PWD)/$(INSTALL_HOME)/package/Contents/tmp/Akua_2009Q4_Release/images.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2009Q4_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2009Q4_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_classic.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2009Q4_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_crystal.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2009Q4_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_hicontrast.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2009Q4_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_industrial.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua_2009Q4_Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/basis-link/share/config/images_tango.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/NeoOfficeAquaElements.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cd "$(INSTALL_HOME)/package/Contents" ; cp "tmp/NeoOffice Aqua Elements 3/Contents/MacOS/"*.bmp "program"
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
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libMacOSXSpell$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/classes/avmedia.jar'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libavmediaQuickTime$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libavmediaquicktime.dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libdtransaqua$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libdtransjava$(DLLSUFFIX).dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/fps_aqua.uno.dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/fps_java.uno.dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/libmozab2.dylib'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -register -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/updchk.uno.dylib'
# Do not ship the Lotus Word Pro filter as it is very unstable on Mac OS X
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/basis-link/program" ; regcomp -revoke -r services.rdb -c 'vnd.sun.star.expand:$$OOO_BASE_DIR/program/liblwpft$(DLLSUFFIX).dylib'
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names" | sed "s#-#_#g"` ; do mkdir -p "$${i}.lproj" ; mkdir -p `echo "$${i}" | sed "s#_.*\\$$##"`".lproj" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "basis-link/program/open-url" LICENSE* README* licenses/* share/readme/*
# Fix bug 3273 by not installing any OOo or ooo-build fonts
	cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "basis-link/program/libMacOSXSpell$(DLLSUFFIX).dylib" "basis-link/program/libavmediaQuickTime$(DLLSUFFIX).dylib" "basis-link/program/libdtransaqua$(DLLSUFFIX).dylib" "basis-link/program/fps_aqua.uno.dylib" "basis-link/program/liblwpft$(DLLSUFFIX).dylib" "basis-link/share/fonts/truetype" "basis-link/share/psprint"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/gpl.html" "share/readme/LICENSE_en-US.html"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/gpl.txt" "share/readme/LICENSE_en-US"
	cd "$(INSTALL_HOME)/package/Contents/basis-link" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share -type f | grep -v /CVS | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents/basis-link" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share -type f | grep -v /CVS` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#'  | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' > "../../out" ; mv -f "../../out" "MacOS/bootstraprc"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/program/fundamentalrc" "MacOS/fundamentalrc"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/program/jvmfwk3rc" "basis-link/ure-link/lib/jvmfwk3rc"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/program/versionrc" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_UPDATE_CHECK_URL)#$(PRODUCT_UPDATE_CHECK_URL)#g' | sed 's# #%20#g' | sed 's#^buildid=.*$$#buildid=$(PRODUCT_PATCH_VERSION)#' > "MacOS/versionrc"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in "share/registry/data/org/openoffice/Office/Compatibility.xcu" "share/registry/modules/org/openoffice/Office/Common/Common-brand.xcu" "share/registry/modules/org/openoffice/Office/UI/UI-brand.xcu" "share/registry/modules/org/openoffice/Setup/Setup-brand.xcu" ; do sed "s#oor:name=\"$(OO_PRODUCT_NAME)\"#oor:name=\"$(PRODUCT_NAME)\"#g" "$${i}" | sed "s#>$(OO_PRODUCT_NAME)<#>$(PRODUCT_NAME)<#g" | sed "s#>$(PRODUCT_VERSION_FAMILY)<#>$(PRODUCT_VERSION)<#g" | sed "s#>$(OO_PRODUCT_VERSION)<#>$(PRODUCT_VERSION)<#g" | sed "s#>$(OO_REGISTRATION_URL)<#>$(PRODUCT_REGISTRATION_URL)<#g" > "../../out" ; mv -f "../../out" "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(OO_PRODUCT_NAME)#$(OO_PRODUCT_NAME)#g' "$(PWD)/etc/help/main_transform.xsl" | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOWNLOAD_URL)#$(PRODUCT_DOWNLOAD_URL)#g' | sed 's#$$(PRODUCT_DOWNLOAD_URL_TEXT)#$(PRODUCT_DOWNLOAD_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOWNLOADLANGPACK_URL)#$(PRODUCT_DOWNLOADLANGPACK_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL)#$(PRODUCT_DOCUMENTATION_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_URL_TEXT)#$(PRODUCT_DOCUMENTATION_URL_TEXT)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#$(PRODUCT_DOCUMENTATION_LAUNCHSHORTCUTS_URL)#g' | sed 's#$$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#$(PRODUCT_DOCUMENTATION_SPELLCHECK_URL)#g' > "basis-link/help/main_transform.xsl"
# With gcc 4.x, we must fully strip executables
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.bin"` ; do strip "$$i" ; done'
ifeq ("$(OS_MAJOR_VERSION)","10.4")
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*"` ; do strip -S -x "$$i" ; done'
else
# Mac OS 10.5.x and higher cannot strip the Mozilla libraries to exclude them
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*" | grep -v "components"` ; do strip -S -x "$$i" ; done'
endif
# Integrate the iMediaBrowser framework
	mkdir -p "$(INSTALL_HOME)/package/Contents/Frameworks"
	cd "$(INSTALL_HOME)/package" ; ( ( cd "$(PWD)/$(BUILD_HOME)/$(IMEDIA_PACKAGE)/build/Debug" ; gnutar cvf - --exclude Headers --exclude PrivateHeaders iMediaBrowser.framework ) | ( cd "$(PWD)/$(INSTALL_HOME)/package/Contents/Frameworks" ; gnutar xvf - ; strip -S -x iMediaBrowser.framework/Versions/A/iMediaBrowser ) )
# Integrate the RemoteControl framework
	mkdir -p "$(INSTALL_HOME)/package/Contents/Frameworks"
	cd "$(INSTALL_HOME)/package" ; ( ( cd "$(PWD)/$(BUILD_HOME)/$(REMOTECONTROL_PACKAGE)/build/Release" ; gnutar cvf - --exclude Headers RemoteControl.framework ) | ( cd "$(PWD)/$(INSTALL_HOME)/package/Contents/Frameworks" ; gnutar xvf - ; strip -S -x RemoteControl.framework/Versions/A/RemoteControl ) )
# Install OOo .oxt files. Note that we exclude the wiki-publisher.oxt file as
# has been found to have buggy network connectivity. Fix bug 3434 by using our
# patched build of presenter-screen.oxt. Fix bug 3501 by using our patched
# build of sun-presentation-minimizer.oxt.
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -c -e 'JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1 ; export JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY ; unset CLASSPATH ; unset DYLD_LIBRARY_PATH ; for i in `find "$(PWD)/$(BUILD_HOME)/solver/$${UPD}/$(UOUTPUTDIR)/bin" -type f -name "*.oxt" | grep -v "wiki-publisher.oxt" | grep -v "presenter-screen.oxt" | grep -v "sun-presentation-minimizer.oxt"` "$(PWD)/$(BUILD_HOME)/sdext/$(UOUTPUTDIR)/bin/presenter-screen.oxt" "$(PWD)/$(BUILD_HOME)/sdext/$(UOUTPUTDIR)/bin/sun-presentation-minimizer.oxt" ; do rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp" ; echo "yes" | ./unopkg.bin add --shared --verbose "$$i" -env:UserInstallation=file://"$(PWD)/$(INSTALL_HOME)/tmp" ; done ; rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp"'
# Install shared .oxt files
	cd "$(INSTALL_HOME)/package/Contents/MacOS" ; sh -c -e 'JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY=1 ; export JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY ; unset CLASSPATH ; unset DYLD_LIBRARY_PATH ; for i in `echo "$(PRODUCT_COMPONENT_MODULES)"` ; do if [ -f "$(PWD)/$$i/$(UOUTPUTDIR)/bin/$$i.oxt" ] ; then rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp" ; ./unopkg.bin add --shared --verbose "$(PWD)/$$i/$(UOUTPUTDIR)/bin/$$i.oxt" -env:UserInstallation=file://"$(PWD)/$(INSTALL_HOME)/tmp" ; fi ; done ; rm -Rf "$(PWD)/$(INSTALL_HOME)/tmp"'
# Integrate the odf-converter. Don't strip the binaries as it will break the
# Mono libraries
	mkdir -p "$(INSTALL_HOME)/package/Contents/MacOS/mono/2.0"
	cd "$(INSTALL_HOME)/package" ; ( ( cd "$(PWD)/$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; gnutar cvf - . ) | ( cd "$(PWD)/$(INSTALL_HOME)/package/Contents/MacOS" ; gnutar xvf - ) )
	mkdir -p "$(INSTALL_HOME)/package/Contents/Library/Spotlight"
	cd "$(INSTALL_HOME)/package/Contents/Library/Spotlight" ; curl -L "$(NEOLIGHT_MDIMPORTER_URL)" | tar zxvf -
#	Make Spotlight plugin ID unique for each build
	cd "$(INSTALL_HOME)/package/Contents/Library/SpotLight" ; sed 's#$(NEOLIGHT_MDIMPORTER_ID)#$(NEOLIGHT_MDIMPORTER_ID).$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)#g' "neolight.mdimporter/Contents/Info.plist" > "../../out" ; mv -f "../../out" "neolight.mdimporter/Contents/Info.plist"
	mkdir -p "$(INSTALL_HOME)/package/Contents/Library/QuickLook"
	cd "$(INSTALL_HOME)/package/Contents/Library/QuickLook" ; curl -L "$(NEOPEEK_QLPLUGIN_URL)" | tar zxvf -
#	Make QL plugin ID unique for each build
	cd "$(INSTALL_HOME)/package/Contents/Library/QuickLook" ; sed 's#$(NEOPEEK_QLPLUGIN_ID)#$(NEOPEEK_QLPLUGIN_ID).$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)#g' "neopeek.qlgenerator/Contents/Info.plist" > "../../out" ; mv -f "../../out" "neopeek.qlgenerator/Contents/Info.plist"
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find "." -name ".DS_Store"` ; do rm "$${i}" ; done'
	chmod -Rf a-w,a+r "$(INSTALL_HOME)/package"
	echo "Running sudo to chown installation files..."
	sudo chown -Rf root:admin "$(INSTALL_HOME)/package"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -e -c 'for i in `cd "/Applications/Utilities/Installer.app/Contents/Resources" ; find . -type d -name "*.lproj" -maxdepth 1` ; do mkdir -p "$${i}" ; done'
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; cp "$(PWD)/etc/package/ship.tiff" "background.tiff"
	printf "pmkrpkg1" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/PkgInfo"
	( cd "$(INSTALL_HOME)/package" ; pax -w -z -x cpio . ) > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Archive.pax.gz"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Info.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Info.plist"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Description.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/Description.plist"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../Description.plist" "$${i}/Description.plist" ; done'
	mkbom "$(INSTALL_HOME)/package" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Archive.bom" >& /dev/null
	cp "etc/gpl.html" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/License.html"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/ReadMe.rtf" | sed 's#$$(PRODUCT_TRADEMARKED_NAME_RTF)#'"$(PRODUCT_TRADEMARKED_NAME_RTF)"'#g' | sed 's#$$(PRODUCT_BASE_URL)#'"$(PRODUCT_BASE_URL)"'#g' | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/ReadMe.rtf"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(TARGET_FILE_TYPE)#$(TARGET_FILE_TYPE)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/installutils"
	sed 's#$$(TARGET_MACHINE)#$(UNAME)#g' "bin/InstallationCheck" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/InstallationCheck" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/InstallationCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/InstallationCheck.strings" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/InstallationCheck.strings"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../InstallationCheck.strings" "$${i}/InstallationCheck.strings" ; done'
	cp "bin/preflight" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/preflight" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/preflight"
	sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' "bin/postflight" | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(NEOLIGHT_MDIMPORTER_ID)#$(NEOLIGHT_MDIMPORTER_ID).$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)#g' | sed 's#$$(NEOPEEK_QLPLUGIN_ID)#$(NEOPEEK_QLPLUGIN_ID).$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/postflight" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/postflight"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	mv -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	cp -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).pkg/Contents/Resources/ReadMe.rtf" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/ReadMe.rtf"
# No longer include source in distribution as the source has grown too large
#	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/src"
#	cp -f "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION).src.tar.gz" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/src"
	chmod -Rf a-w,a+r "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	chmod -f u+w "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	mv "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
	sync ; hdiutil create -srcfolder "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)" -format UDZO -ov -o "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).dmg"

build.odk_package: build.neo_odk_patches
	touch "$@"

build.patch_package: build.package
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	"$(MAKE)" $(MFLAGS) "build.patch_package_shared"
	touch "$@"

build.patch_package_shared:
	sh -e -c 'if [ -d "$(PATCH_INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(PATCH_INSTALL_HOME)" ; fi'
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/MacOS"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/basis-link/program/classes"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/basis-link/ure-link/lib"
	chmod -Rf u+w,a+r "$(PATCH_INSTALL_HOME)/package"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/fpicker/$(UOUTPUTDIR)/lib/fps_java.uno.dylib" "$(PWD)/$(BUILD_HOME)/package/$(UOUTPUTDIR)/lib/libxstor.dylib" "$(PWD)/$(BUILD_HOME)/sc/$(UOUTPUTDIR)/lib/libsc$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/sfx2/$(UOUTPUTDIR)/lib/libsfx$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/svtools/$(UOUTPUTDIR)/lib/libsvt$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/lib/libsvx$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/lib/libvcl$(DLLSUFFIX).dylib" "basis-link/program"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/lib/sunjavaplugin.dylib" "$(PWD)/$(BUILD_HOME)/sal/$(UOUTPUTDIR)/lib/libuno_sal.dylib.3" "basis-link/ure-link/lib"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/package/Info.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME)#$(PRODUCT_TRADEMARKED_NAME)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g'  | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PRODUCT_FILETYPE)#$(PRODUCT_FILETYPE)#g' > "Info.plist"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/class/vcl.jar" "basis-link/program/classes"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#'  | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' > "../../out" ; mv -f "../../out" "MacOS/bootstraprc"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/program/versionrc" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_UPDATE_CHECK_URL)#$(PRODUCT_UPDATE_CHECK_URL)#g' | sed 's# #%20#g' | sed 's#^buildid=.*$$#buildid=$(PRODUCT_PATCH_VERSION)#' > "MacOS/versionrc"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*"` ; do strip -S -x "$$i" ; done'
	chmod -Rf a-w,a+r "$(PATCH_INSTALL_HOME)/package"
	echo "Running sudo to chown installation files..."
	sudo chown -Rf root:admin "$(PATCH_INSTALL_HOME)/package"
	mkdir -p "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources"
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -e -c 'for i in `cd "/Applications/Utilities/Installer.app/Contents/Resources" ; find . -type d -name "*.lproj" -maxdepth 1` ; do mkdir -p "$${i}" ; done'
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; cp "$(PWD)/etc/package/ship.tiff" "background.tiff"
	printf "pmkrpkg1" > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/PkgInfo"
	( cd "$(PATCH_INSTALL_HOME)/package" ; pax -w -z -x cpio . ) > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Archive.pax.gz"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Info.plist.patch" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Info.plist"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Description.plist.patch" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/Description.plist"
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../Description.plist" "$${i}/Description.plist" ; done'
# Copy shared .oxt files
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -c -e 'for i in `echo "$(PRODUCT_COMPONENT_PATCH_MODULES)"` ; do if [ -f "$(PWD)/$$i/$(UOUTPUTDIR)/bin/$$i.oxt" ] ; then cp "$(PWD)/$$i/$(UOUTPUTDIR)/bin/$$i.oxt" . ; fi ; done'
# Make empty BOM so that nothing gets extracted in the temporary installation
	mkdir "$(PATCH_INSTALL_HOME)/emptydir"
	mkbom "$(PATCH_INSTALL_HOME)/emptydir" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Archive.bom" >& /dev/null
	rm -Rf "$(PATCH_INSTALL_HOME)/emptydir"
	cp "etc/gpl.html" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/License.html"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/ReadMe.rtf" | sed 's#$$(PRODUCT_TRADEMARKED_NAME_RTF)#'"$(PRODUCT_TRADEMARKED_NAME_RTF)"'#g' | sed 's#$$(PRODUCT_BASE_URL)#'"$(PRODUCT_BASE_URL)"'#g' | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/ReadMe.rtf"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils.patch" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PREVIOUS_PRODUCT_VERSION)#$(PREVIOUS_PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(TARGET_FILE_TYPE)#$(TARGET_FILE_TYPE)#g' | sed 's#$$(NEOLIGHT_MDIMPORTER_ID)#$(NEOLIGHT_MDIMPORTER_ID).$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)#g' | sed 's#$$(NEOPEEK_QLPLUGIN_ID)#$(NEOPEEK_QLPLUGIN_ID).$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/installutils"
	sed 's#$$(TARGET_MACHINE)#$(UNAME)#g' "bin/InstallationCheck" > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/InstallationCheck" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/InstallationCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/InstallationCheck.strings" > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/InstallationCheck.strings"
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../InstallationCheck.strings" "$${i}/InstallationCheck.strings" ; done'
	cp "bin/VolumeCheck.patch" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/VolumeCheck" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/VolumeCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/VolumeCheck.strings.patch" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/VolumeCheck.strings"
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do cp "VolumeCheck.strings" "$${i}/VolumeCheck.strings" ; done' ; rm -f "VolumeCheck.strings"
	cp "bin/preflight.patch" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/preflight" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/preflight"
	cp "bin/postflight.patch" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/postflight" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/postflight"
	mkdir -p "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	mv -f "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	cp -f "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).pkg/Contents/Resources/ReadMe.rtf" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)/ReadMe.rtf"
	chmod -Rf a-w,a+r "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	chmod -f u+w "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	mv "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
	sync ; hdiutil create -srcfolder "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)" -format UDZO -ov -o "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).dmg"

build.package_%: $(INSTALL_HOME)/package_%
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	chmod -Rf u+w,a+r "$<"
# Fix bug 3426 by including the legacy binfilter files
	cd "$</Contents" ; sh -c -e 'for i in `cd "$(PWD)/$(BUILD_HOME)/binfilter/$(UOUTPUTDIR)/bin" ; find . -name "*$(PRODUCT_LANG_PACK_LOCALE).res" -maxdepth 1` ; do cp "$(PWD)/$(BUILD_HOME)/binfilter/$(UOUTPUTDIR)/bin/$${i}" "basis-link/program/resource/$${i}" ; done'
	cd "$</Contents" ; cp "$(PWD)/$(BUILD_HOME)/filter/$(UOUTPUTDIR)/misc/filters/ui/langpacks/$(PRODUCT_LANG_PACK_LOCALE)/org/openoffice/TypeDetection/Filter.xcu" "basis-link/share/registry/res/$(PRODUCT_LANG_PACK_LOCALE)/org/openoffice/TypeDetection/Filter.xcu"
	cd "$</Contents" ; rm -Rf LICENSE* README* licenses/* share/readme/*
	cd "$</Contents" ; cp "$(PWD)/etc/gpl.html" "share/readme/LICENSE_$(PRODUCT_LANG_PACK_LOCALE).html"
	cd "$</Contents" ; cp "$(PWD)/etc/gpl.txt" "share/readme/LICENSE_$(PRODUCT_LANG_PACK_LOCALE)"
	rm -Rf "$</Contents/Resources"
	mkdir -p "$</Contents/Resources"
	cd "$<" ; sh -e -c 'for i in `find "." -name ".DS_Store"` ; do rm "$${i}" ; done'
	chmod -Rf a-w,a+r "$<"
	echo "Running sudo to chown $(@:build.package_%=%) installation files..."
	sudo chown -Rf root:admin "$<"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -e -c 'for i in `cd "/Applications/Utilities/Installer.app/Contents/Resources" ; find . -type d -name "*.lproj" -maxdepth 1` ; do mkdir -p "$${i}" ; done'
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; cp "$(PWD)/etc/package/ship.tiff" "background.tiff"
	printf "pmkrpkg1" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/PkgInfo"
	( cd "$<" ; pax -w -z -x cpio . ) > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Archive.pax.gz"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Info.plist.langpack" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_LANG_PACK_VERSION)#$(PRODUCT_LANG_PACK_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Info.plist"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Description.plist.langpack" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_LANG_PACK_VERSION)#$(PRODUCT_LANG_PACK_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/Description.plist"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../Description.plist" "$${i}/Description.plist" ; done'
# Make empty BOM so that nothing gets extracted in the temporary installation
	mkdir "$(INSTALL_HOME)/emptydir"
	mkbom "$(INSTALL_HOME)/emptydir" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Archive.bom" >& /dev/null
	rm -Rf "$(INSTALL_HOME)/emptydir"
	cp "etc/gpl.html" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/License.html"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/ReadMe.rtf" | sed 's#$$(PRODUCT_TRADEMARKED_NAME_RTF)#'"$(PRODUCT_TRADEMARKED_NAME_RTF)"'#g' | sed 's#$$(PRODUCT_BASE_URL)#'"$(PRODUCT_BASE_URL)"'#g' | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/ReadMe.rtf"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils.langpack" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(TARGET_FILE_TYPE)#$(TARGET_FILE_TYPE)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/installutils"
	sed 's#$$(TARGET_MACHINE)#$(UNAME)#g' "bin/InstallationCheck" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/InstallationCheck" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/InstallationCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/InstallationCheck.strings" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/InstallationCheck.strings"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../InstallationCheck.strings" "$${i}/InstallationCheck.strings" ; done'
	cp "bin/VolumeCheck.langpack" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/VolumeCheck" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/VolumeCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/VolumeCheck.strings.langpack" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/VolumeCheck.strings"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do cp "VolumeCheck.strings" "$${i}/VolumeCheck.strings" ; done' ; rm -f "VolumeCheck.strings"
	cp "bin/preflight.langpack" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/preflight" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/preflight"
	cp "bin/postflight.langpack" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/postflight" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/postflight"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)"
	mv -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)"
	cp -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).pkg/Contents/Resources/ReadMe.rtf" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)/ReadMe.rtf"
	chmod -Rf a-w,a+r "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)"
	chmod -f u+w "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)"
	mv "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)"
	sync ; hdiutil create -srcfolder "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)" -format UDZO -ov -o "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).dmg"

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

build.all: build.package
	touch "$@"
