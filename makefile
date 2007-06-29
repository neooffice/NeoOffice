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
PRODUCT_NAME=My Untested Office Suite
PRODUCT_DIR_NAME=My_Untested_Office_Suite
# Important: Note that there may be escape characters in the PRODUCT_NAME for
# the UTF-8 trademark symbol. Don't replace these with "\x##" literal strings!
PRODUCT_TRADEMARKED_NAME=$(PRODUCT_NAME)
PRODUCT_TRADEMARKED_NAME_RTF=$(PRODUCT_NAME)

# Custom overrides go in the following file
-include custom.mk

# Set the shell to tcsh since the OpenOffice.org build requires it
SHELL:=/bin/tcsh
UNAME:=$(shell uname -p)
ifeq ("$(UNAME)","powerpc")
ULONGNAME=PowerPC
UOUTPUTDIR=unxmacxp.pro
DLLSUFFIX=mxp
TARGET_FILE_TYPE=Mach-O executable ppc
else
ULONGNAME=Intel
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
ifeq ("$(UNAME)","powerpc")
OO_ENV_X11:=$(BUILD_HOME)/MacOSXPPCEnv.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacOSXPPCEnvJava.Set
else
OO_ENV_X11:=$(BUILD_HOME)/MacOSXX86Env.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacOSXX86EnvJava.Set
endif
OO_LANGUAGES=ALL
NEOLIGHT_MDIMPORTER_URL:=http://trinity.neooffice.org/downloads/neolight.mdimporter.tgz
NEOPEEK_QLPLUGIN_URL:=http://trinity.neooffice.org/downloads/neopeek.qlgenerator.tgz

# Product information
OO_PRODUCT_NAME=OpenOffice.org
OO_PRODUCT_VERSION=2.2
OO_REGISTRATION_URL=http://www.openoffice.org/welcome/registration20.html
OO_SUPPORT_URL=http://www.openoffice.org
OO_SUPPORT_URL_TEXT=www.openoffice.org
PRODUCT_VERSION_FAMILY=2.2
PRODUCT_VERSION=2.2.1
PRODUCT_DIR_VERSION=2.2.1
PRODUCT_LANG_PACK_VERSION=Language Pack
PRODUCT_DIR_LANG_PACK_VERSION=Language_Pack
PRODUCT_PATCH_VERSION=Patch 0
PRODUCT_DIR_PATCH_VERSION=Patch-0
PRODUCT_REGISTRATION_URL=http://trinity.neooffice.org/modules.php?name=Your_Account\&amp\;redirect=index
PRODUCT_SUPPORT_URL=http://trinity.neooffice.org/modules.php?name=Forums
PRODUCT_SUPPORT_URL_TEXT:=$(PRODUCT_NAME) Support

# CVS macros
OO_CVSROOT:=:pserver:anoncvs@anoncvs.services.openoffice.org:/cvs
OO_PACKAGES:=OpenOffice2
OO_TAG:=-rOpenOffice_2_2_1
OOO-BUILD_SVNROOT:=http://svn.gnome.org/svn/ooo-build/tags/OOO_BUILD_2_2_1
OOO-BUILD_PACKAGE:=ooo-build
OOO-BUILD_TAG:=
OOO-BUILD_APPLY_TAG:=OOF680_m18
LPSOLVE_SOURCE_URL=http://go-ooo.org/packages/SRC680/lp_solve_5.5.tar.gz
LIBWPD_SOURCE_URL=http://go-ooo.org/packages/libwpd/libwpd-0.8.10.tar.gz
LIBWPG_SOURCE_URL=http://go-ooo.org/packages/SRC680/libwpg-0.1.0~cvs20070608.tar.gz
LIBWPS_SOURCE_URL=http://go-ooo.org/packages/SRC680/libwps-0.1.0~svn20070129.tar.gz
XT_SOURCE_URL=http://go-ooo.org/packages/xt/xt-20051206-src-only.zip
MOZ_SOURCE_URL=ftp://ftp.mozilla.org/pub/mozilla.org/mozilla/releases/mozilla1.7.5/source/mozilla-source-1.7.5.tar.gz
ODF-CONVERTER_SVNROOT=https://odf-converter.svn.sourceforge.net/svnroot/odf-converter/trunk
ODF-CONVERTER_PACKAGE=odf-converter
ODF-CONVERTER_TAG:=--revision '{2007-06-23}'
NEO_CVSROOT:=:pserver:anoncvs@anoncvs.neooffice.org:/cvs
NEO_PACKAGE:=NeoOffice
NEO_TAG:=-rHEAD

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

build.odf-converter_checkout:
	rm -Rf "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)"
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; svn co $(ODF-CONVERTER_TAG) $(ODF-CONVERTER_SVNROOT) "$(ODF-CONVERTER_PACKAGE)"
	cd "$(BUILD_HOME)" ; chmod -Rf u+w "$(ODF-CONVERTER_PACKAGE)"
# odf-converter engineers seem to not know that creating a file on Windows and
# then checking it into cvs or svn from a Unix machine foobar's the newlines
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/source" ; sh -e -c 'for i in `find . -name "*.xsl"` ; do cat "$${i}" | tr -d "\015" > "../out" ; mv -f "../out" "$${i}" ; done'
	touch "$@"

build.oo_patches: build.ooo-build_patches \
	build.oo_automation_patch \
	build.oo_berkeleydb_patch \
	build.oo_binfilter_patch \
	build.oo_chart2_patch \
	build.oo_config_office_patch \
	build.oo_external_patch \
	build.oo_forms_patch \
	build.oo_framework_patch \
	build.oo_instsetoo_native_patch \
	build.oo_jvmfwk_patch \
	build.oo_lingucomponent_patch \
	build.oo_moz_patch \
	build.oo_padmin_patch \
	build.oo_sc_patch \
	build.oo_sj2_patch \
	build.oo_solenv_patch \
	build.oo_store_patch \
	build.oo_sw_patch \
	build.oo_toolkit_patch \
	build.oo_ucb_patch \
	build.oo_vcl_patch \
	build.oo_vos_patch
# Copy modified compiler scripts to work around gcc 3.3 breakage in Apple's
# latest system updates
	mkdir -p "$(COMPILERDIR)"
	cd "$(COMPILERDIR)" ; sh -c -e 'for i in cc gcc c++ g++ ; do cp "$(PWD)/$(OO_PATCHES_HOME)/cc" "$$i" ; chmod 755 "$$i"; done'
	touch "$@"

build.oo_odk_patches: build.oo_patches
	touch "$@"

build.oo_external_patch: build.ooo-build_patches
	chmod -Rf u+w "$(BUILD_HOME)/external/gpc"
	gnutar zxf "$(OO_PATCHES_HOME)/gpc231.tar.Z" -C "$(BUILD_HOME)/external/gpc"
	chmod -Rf u+w "$(BUILD_HOME)/external/gpc"
	mv -f "$(BUILD_HOME)/external/gpc/gpc231"/* "$(BUILD_HOME)/external/gpc"
	rm -Rf "$(BUILD_HOME)/external/gpc/gpc231"
	touch "$@"

build.oo_moz_patch: build.ooo-build_patches
	cd "$(BUILD_HOME)/moz/download" ; curl -O "$(MOZ_SOURCE_URL)"
	touch "$@"

build.oo_%_patch: $(OO_PATCHES_HOME)/%.patch build.ooo-build_patches
	-( cd "$(BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.ooo-build_patches: build.ooo-build_checkout \
	build.ooo-build_apply_patch \
	build.ooo-build_svx_patch
	touch "$@"

build.ooo-build_apply_patch: $(OOO-BUILD_PATCHES_HOME)/apply.patch build.oo_checkout build.ooo-build_checkout
	-( cd "$(BUILD_HOME)/ooo-build" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/ooo-build" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	"$(BUILD_HOME)/ooo-build/patches/apply.pl" --tag="$(OOO-BUILD_APPLY_TAG)" --distro=MacOSX "$(PWD)/$(BUILD_HOME)/ooo-build/patches/src680" "$(PWD)/$(BUILD_HOME)"
	cp "$(BUILD_HOME)/ooo-build/src/go-oo-team.png" "$(BUILD_HOME)/default_images/sw/res"
	cp "$(BUILD_HOME)/ooo-build/src/evolocal.odb" "$(BUILD_HOME)/extras/source/database"
	mkdir -p "$(BUILD_HOME)/lpsolve/download" ; cd "$(BUILD_HOME)/lpsolve/download" ; curl -O "$(LPSOLVE_SOURCE_URL)"
	mkdir -p "$(BUILD_HOME)/libwpd/download" ; cd "$(BUILD_HOME)/libwpd/download" ; curl -O "$(LIBWPD_SOURCE_URL)"
	mkdir -p "$(BUILD_HOME)/libwpg/download" ; cd "$(BUILD_HOME)/libwpg/download" ; curl -O "$(LIBWPG_SOURCE_URL)"
	mkdir -p "$(BUILD_HOME)/libwps/download" ; cd "$(BUILD_HOME)/libwps/download" ; curl -O "$(LIBWPS_SOURCE_URL)"
	cd "$(BUILD_HOME)/xt/download" ; curl -O "$(XT_SOURCE_URL)"
	touch "$@"

build.ooo-build_%_patch: $(OOO-BUILD_PATCHES_HOME)/%.patch build.oo_checkout build.ooo-build_checkout
	-( cd "$(BUILD_HOME)/$(@:build.ooo-build_%_patch=%)" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(@:build.ooo-build_%_patch=%)" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.odf-converter_patches: $(ODF-CONVERTER_PATCHES_HOME)/odf-converter.patch build.odf-converter_checkout
	-( cd "$(BUILD_HOME)/odf-converter" ; patch -b -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/odf-converter" ; patch -b -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)" ; setenv PATH "$(PWD)/$(COMPILERDIR)":/usr/bin:"$$PATH" ; "$(MAKE)" $(MFLAGS)
	rm -Rf "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist"
	mkdir -p "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist"
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; ( ( cd "`/usr/bin/pkg-config --variable=prefix mono`/etc" ; gnutar cvf - mono ) | ( cd "$(PWD)/$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; gnutar xvf - ) )
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; cp "$(PWD)/$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/source/Shell/OdfConverterTest/OdfConverter" "OdfConverter" ; chmod a+x "OdfConverter" ; otool -L "OdfConverter" | awk '{ print $$1 }' | grep '\.dylib' | grep -v ':$$' | grep -v '@executable_path\/' | grep -v '^\/usr\/lib\/' | grep -v '^\/System\/Library\/Frameworks\/' > "library.list"
# Find all non-system linked libraries
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; touch "library.list.bak" ; sh -c -e 'while ! diff -q "library.list" "library.list.bak" >/dev/null ; do cp "library.list" "library.list.bak" ; for i in `cat "library.list"` ; do otool -L "$$i" | awk "{ print \$$1 }" | grep "\.dylib" | grep -v ":\$$" | grep -v "@executable_path\/" | grep -v "^\/usr\/lib\/" | grep -v "^\/System\/Library\/Frameworks\/" >> "library.list" ; done ; sort -u "library.list" > "library.list.tmp" ; mv "library.list.tmp" "library.list" ; done' ; rm "library.list.bak"
# Resolve and copy all softlinks
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; touch "library.list.bak" ; sh -c -e 'while ! diff -q "library.list" "library.list.bak" >/dev/null ; do cp "library.list" "library.list.bak" ; for i in `cat "library.list"` ; do if [ -h "$$i" ] ; then linkedfile=`ls -l "$$i" | awk "{ print \\$$NF }"` ; dirname=`dirname "$$i"` ; if [ -f "$$dirname/$$linkedfile" ] ; then echo "$$dirname/$$linkedfile" >> "library.list" ; ln -sf "$$linkedfile" `basename "$$i"` ; fi ; fi ; done ; sort -u "library.list" > "library.list.tmp" ; mv "library.list.tmp" "library.list" ; done' ; rm "library.list.bak"
# Copy all files
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; sh -c -e 'for i in `cat "library.list"` ; do if [ -f "$$i" ] ; then cp "$$i" `basename "$$i"` ; fi ; done'
# Change each library's internal name to @executable_path/libname
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; sh -c -e 'for i in OdfConverter `cat "library.list"` ; do basename=`basename "$$i"` ; install_name_tool -id "@executable_path/$$basename" "$$basename" ; done'
# Change each library's link list to @executable_path/linkedlibname
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; sh -c -e 'for i in OdfConverter `cat "library.list"` ; do for j in `cat "library.list"` ; do basename=`basename "$$i"` ; install_name_tool -change "$$j" @executable_path/`basename "$$j"` "$$basename" ; done ; done'
	cd "$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; rm "library.list"
	touch "$@"

build.configure: build.oo_patches
	cd "$(BUILD_HOME)/config_office" ; autoconf
	( cd "$(BUILD_HOME)/config_office" ; setenv PATH "$(PWD)/$(COMPILERDIR):/bin:/sbin:/usr/bin:/usr/sbin:$(EXTRA_PATH)" ; unsetenv DYLD_LIBRARY_PATH ; ./configure CC=$(CC) CXX=$(CXX) PKG_CONFIG=$(PKG_CONFIG) --with-jdk-home=/System/Library/Frameworks/JavaVM.framework/Home --with-java-target-version=1.4 --with-epm=internal --enable-vba --disable-cups --disable-gtk --disable-odk --without-nas --with-mozilla-toolkit=xlib --with-gnu-cp="$(GNUCP)" --with-system-curl --without-system-mdbtools --with-x --x-includes=/usr/X11R6/include --with-lang="$(OO_LANGUAGES)" )
	echo 'setenv LIBIDL_CONFIG "$(LIBIDL_CONFIG)"' >> "$(OO_ENV_X11)"
	echo 'setenv PKG_CONFIG "$(PKG_CONFIG)"' >> "$(OO_ENV_X11)"
	echo 'unsetenv LD_SEG_ADDR_TABLE' >> "$(OO_ENV_X11)"
	echo 'unsetenv LD_PREBIND' >> "$(OO_ENV_X11)"
	echo 'unsetenv LD_PREBIND_ALLOW_OVERLAP' >> "$(OO_ENV_X11)"
	( cd "$(BUILD_HOME)" ; ./bootstrap )
	touch "$@"

build.oo_all: build.configure
	source "$(OO_ENV_X11)" ; cd "$(BUILD_HOME)/instsetoo_native" ; `alias build` --all $(OO_BUILD_ARGS)
	touch "$@"

build.oo_odk_all: build.configure build.oo_all build.oo_odk_patches
	source "$(OO_ENV_X11)" ; cd "$(BUILD_HOME)/odk" ; `alias build` $(OO_BUILD_ARGS)
	touch "$@"

build.neo_configure: build.oo_all neo_configure.mk
	rm -f "$(OO_ENV_JAVA)"
	$(MAKE) $(MFLAGS) build.neo_configure_phony
	touch "$@"

build.neo_%_patch: % build.neo_configure
	cd "$<" ; sh -e -c 'for i in `cd "$(PWD)/$(BUILD_HOME)/$<" ; find . -type d | grep -v /CVS$$ | grep -v /$(UOUTPUTDIR)` ; do mkdir -p "$$i" ; done'
	cd "$<" ; sh -e -c 'for i in `cd "$(PWD)/$(BUILD_HOME)/$<" ; find . ! -type d | grep -v /CVS/ | grep -v /$(UOUTPUTDIR)` ; do if [ ! -f "$$i" ] ; then ln -sf "$(PWD)/$(BUILD_HOME)/$</$$i" "$$i" 2>/dev/null ; fi ; done'
	sh -e -c 'if [ ! -d "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR).oo" -a -d "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR)" ] ; then rm -Rf "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR).oo" ; mv -f "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR)" "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR).oo" ; fi'
	rm -Rf "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR)"
	mkdir -p "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR)"
	cd "$<" ; ln -sf "$(PWD)/$(BUILD_HOME)/$</$(UOUTPUTDIR)"
	source "$(OO_ENV_JAVA)" ; cd "$<" ; `alias build` $(NEO_BUILD_ARGS)
	touch "$@"

build.neo_patches: build.oo_all build.odf-converter_patches \
	build.neo_avmedia_patch \
	build.neo_canvas_patch \
	build.neo_desktop_patch \
	build.neo_dtrans_patch \
	build.neo_fpicker_patch \
	build.neo_framework_patch \
	build.neo_hsqldb_patch \
	build.neo_jvmfwk_patch \
	build.neo_sal_patch \
	build.neo_sfx2_patch \
	build.neo_shell_patch \
	build.neo_store_patch \
	build.neo_svtools_patch \
	build.neo_svx_patch \
	build.neo_vcl_patch \
	build.neo_ucbhelper_patch
	touch "$@"

build.neo_odk_patches: \
	build.oo_odk_all \
	build.neo_odk_patch
	touch "$@"

build.package: build.neo_patches build.source_zip
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	sh -e -c 'if [ -d "$(INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(INSTALL_HOME)" ; fi'
	mkdir -p "$(INSTALL_HOME)/package/Contents"
	cd "$(INSTALL_HOME)/package" ; ( ( cd "$(PWD)/$(BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice/install/en-US/staging/OpenOffice.org 2.1.app/Contents/MacOS" ; gnutar cvf - . ) | ( cd "$(PWD)/$(INSTALL_HOME)/package/Contents" ; gnutar xvf - ) )
# Regroup the OOo langauge packs
	cd "$(BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/install" ; find . -type d -maxdepth 1 -exec basename {} \; | grep -v '^\.$$' | grep -v '^log$$' | grep -v '_download$$' > "$(PWD)/$(INSTALL_HOME)/language_names"
ifdef NOLANGPACKS
# Bypass the language pack installers
else
# Create the language pack installers
	source "$(OO_ENV_JAVA)" ; sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names"` ; do if [ "$${i}" = "en-US" ] ; then continue ; fi ; langname=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | awk -F, "{ print \\$$3 }"` ; langdirname=`echo "$${langname}" | sed "s# #_#g"` ; if [ -z "$${langname}" -o -z "$${langdirname}" ] ; then echo "Skipping $${i} language..." ; continue ; fi ; mkdir -p "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents" ; ( ( cd "$(PWD)/$(BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/install/$${i}/staging/OpenOffice.org 2.1.app/Contents/MacOS" ; gnutar cvf - . ) | ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents" ; gnutar xvf - ) ) ; rm -f "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/program/resource/ooo$${UPD}$${i}.res" ; cp "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/bin/ooo$${UPD}$${i}.res" "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/bin/salapp$${UPD}$${i}.res" "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/program/resource" ; helpflag=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | awk -F, "{ print \\$$2 }"` ; if [ "$${helpflag}" != "1" ] ; then rm -Rf "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help/$${i}" ; ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help" ; ln -s "en" "$${i}" ) ; fi ; "$(MAKE)" $(MFLAGS) "PRODUCT_LANG_PACK_LOCALE=$${i}" "PRODUCT_LANG_PACK_VERSION=$(PRODUCT_LANG_PACK_VERSION) $${langname}" "PRODUCT_DIR_LANG_PACK_VERSION=$(PRODUCT_DIR_LANG_PACK_VERSION)_$${langdirname}" "build.package_$${langdirname}" ; done'
endif
	chmod -Rf u+w,a+r "$(INSTALL_HOME)/package"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/avmedia/$(UOUTPUTDIR)/lib/libavmediaquicktime.dylib" "$(PWD)/$(BUILD_HOME)/canvas/$(UOUTPUTDIR)/lib/vclcanvas.uno.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/deployment$${UPD}$(DLLSUFFIX).uno.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/deploymentgui$${UPD}$(DLLSUFFIX).uno.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/migrationoo2.uno.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/libspl$${UPD}$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/dtrans/$(UOUTPUTDIR)/lib/libdtransjava$${UPD}$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/fpicker/$(UOUTPUTDIR)/lib/fpicker.uno.dylib" "$(PWD)/$(BUILD_HOME)/fpicker/$(UOUTPUTDIR)/lib/fps_java.uno.dylib" "$(PWD)/$(BUILD_HOME)/framework/$(UOUTPUTDIR)/lib/libfwk$${UPD}$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/bin/sunjavapluginrc" "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/lib/libjvmfwk.dylib.3" "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/lib/sunjavaplugin.dylib" "$(PWD)/$(BUILD_HOME)/sal/$(UOUTPUTDIR)/lib/libuno_sal.dylib.3" "$(PWD)/$(BUILD_HOME)/sfx2/$(UOUTPUTDIR)/lib/libsfx$${UPD}$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/shell/$(UOUTPUTDIR)/lib/librecentfile.dylib" "$(PWD)/$(BUILD_HOME)/shell/$(UOUTPUTDIR)/lib/localebe1.uno.dylib" "$(PWD)/$(BUILD_HOME)/svtools/$(UOUTPUTDIR)/lib/libsvl$${UPD}$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/svtools/$(UOUTPUTDIR)/lib/libsvt$${UPD}$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/store/$(UOUTPUTDIR)/lib/libstore.dylib.3" "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/lib/libcui$${UPD}$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/lib/libsvx$${UPD}$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/lib/libvcl$${UPD}$(DLLSUFFIX).dylib" "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/lib/libvcljava1.dylib" "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/lib/libvcljava2.dylib" "$(PWD)/$(BUILD_HOME)/ucbhelper/$(UOUTPUTDIR)/lib/libucbhelper3gcc3.dylib" "program"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/bin/salapp$${UPD}en-US.res" "program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/bin/javaldx" "program/javaldx" ; chmod a+x "program/javaldx"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/bin/pkgchk" "program/pkgchk.bin" ; chmod a+x "program/pkgchk.bin"
# With gcc 4.x, we must fully strip the soffice.bin executable
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/bin/soffice" "program/soffice.bin" ; chmod a+x "program/soffice.bin" ; strip "program/soffice.bin"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/resource/ooo$${UPD}"*.res ; cp -f "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/bin/ooo$${UPD}"*.res "program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/shell/$(UOUTPUTDIR)/bin/senddoc" "program/senddoc" ; chmod a+x "program/senddoc"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/bin/javavendors_ooo.xml" "share/config/javavendors.xml"
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/package/Info.plist" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME)#$(PRODUCT_TRADEMARKED_NAME)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PRODUCT_FILETYPE)#$(PRODUCT_FILETYPE)#g' > "Info.plist"
	cd "$(INSTALL_HOME)/package/Contents" ; printf '%s' 'APPL$(PRODUCT_FILETYPE)' > "PkgInfo"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/hsqldb/$(UOUTPUTDIR)/misc/build/hsqldb/lib/hsqldb.jar" "program/classes"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/class/vcl.jar" "program/classes"
	rm -Rf "$(INSTALL_HOME)/package/Contents/Resources"
	mkdir -p "$(INSTALL_HOME)/package/Contents/Resources"
ifeq ("$(PRODUCT_NAME)","NeoOffice")
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/package/ship.icns" "Resources"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/neo2toolbarv10.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_crystal.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Toolbar & Preferences Icons 1.0/images" ; find svtools svx -type f > "$(PWD)/$(INSTALL_HOME)/toolbaricons"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "tmp/NeoOffice Toolbar & Preferences Icons 1.0/source/Generic Template.icns" "Resources/generic.icns"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/NeoOfficeAquaElements.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "tmp/NeoOffice Aqua Elements 2.1.020211/Contents/MacOS/"*.bmp "program"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "tmp/NeoOffice Aqua Elements 2.1.020211/Contents/Resources/"*.icns "Resources"
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 2.1.020211/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 2.1.020211/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_crystal.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"` svtools svx
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 2.1.020211/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/NeoOffice Aqua Elements 2.1.020211/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp" ; unzip "$(PWD)/etc/package/AkuaIcons.zip"
	mkdir -p "$(INSTALL_HOME)/package/Contents/tmp/Akua Beta Release/images"
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua Beta Release/images" ; unzip "$(PWD)/$(INSTALL_HOME)/package/Contents/tmp/Akua Beta Release/images.zip"
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua Beta Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images.zip" .
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua Beta Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_crystal.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_crystal.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua Beta Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_hicontrast.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	cd "$(INSTALL_HOME)/package/Contents/tmp/Akua Beta Release/images" ; touch "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" ; find . -exec touch {} \; ; zip -ru "$(PWD)/$(INSTALL_HOME)/package/Contents/share/config/images_industrial.zip" `cat "$(PWD)/$(INSTALL_HOME)/toolbaricons"`
	chmod -Rf u+rw "$(INSTALL_HOME)/package/Contents/tmp"
	rm -Rf "$(INSTALL_HOME)/package/Contents/tmp"
endif
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names" | sed "s#-#_#g"` ; do mkdir -p "$${i}.lproj" ; mkdir -p `echo "$${i}" | sed "s#_.*\\$$##"`".lproj" ; done'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/program" ; regcomp -revoke -r services.rdb -c "classes/avmedia.jar"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/program" ; regcomp -register -r services.rdb -c "libavmediaquicktime.dylib"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/program" ; regcomp -revoke -r services.rdb -c "libdtransX11$${UPD}$(DLLSUFFIX).dylib"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/program" ; regcomp -register -r services.rdb -c "libdtransjava$${UPD}$(DLLSUFFIX).dylib"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/program" ; regcomp -register -r services.rdb -c "fps_java.uno.dylib"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf "droplet" "program/fondu" "program/gengal" "program/libdtransX11$${UPD}$(DLLSUFFIX).dylib" "program/libpl$${UPD}$(DLLSUFFIX).dylib" "program/libpsp$${UPD}$(DLLSUFFIX).dylib" "program/libspa$${UPD}$(DLLSUFFIX).dylib" "program/libvclplug_gen$${UPD}$(DLLSUFFIX).dylib" "program/open-url" "program/oo_product.bmp" "program/pluginapp.bin" "program/soffice" "program/spadmin" "program/spadmin.bin" "program/testtool.bin" "share/psprint" share/readme/*
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/gpl.html" "share/readme/LICENSE_en-US.html"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/gpl.txt" "share/readme/LICENSE_en-US"
	cd "$(INSTALL_HOME)/package/Contents/program" ; ln -sf "gengal.bin" "gengal"
	cd "$(INSTALL_HOME)/package/Contents/program" ; ln -sf "soffice.bin" "soffice"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find program share -type f | grep -v /CVS | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find program share -type f | grep -v /CVS` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/Library/Preferences/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' > "../../out" ; mv -f "../../out" "program/bootstraprc"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in "share/registry/data/org/openoffice/Setup.xcu" "share/registry/data/org/openoffice/Office/Common.xcu" ; do sed "s#>$(OO_PRODUCT_NAME)<#>$(PRODUCT_NAME)<#g" "$${i}" | sed "s#>$(OO_PRODUCT_VERSION)<#>$(PRODUCT_VERSION)<#g" | sed "s#>$(OO_REGISTRATION_URL)<#>$(PRODUCT_REGISTRATION_URL)<#g" > "../../out" ; mv -f "../../out" "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(OO_PRODUCT_NAME)#$(OO_PRODUCT_NAME)#g' "$(PWD)/etc/help/main_transform.xsl" | sed 's#$$(OO_SUPPORT_URL)#$(OO_SUPPORT_URL)#g' | sed 's#$$(OO_SUPPORT_URL_TEXT)#$(OO_SUPPORT_URL_TEXT)#g' | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' > "help/main_transform.xsl"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*" -o -name "*.bin"` ; do strip -S -x "$$i" ; done'
# Integrate the odf-converter. Don't strip the binaries is it will break the
# Mono libraries
	mkdir -p "$(INSTALL_HOME)/package/Contents/program/mono/2.0"
	cd "$(INSTALL_HOME)/package" ; ( ( cd "$(PWD)/$(BUILD_HOME)/$(ODF-CONVERTER_PACKAGE)/dist" ; gnutar cvf - . ) | ( cd "$(PWD)/$(INSTALL_HOME)/package/Contents/program" ; gnutar xvf - ) )
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'if [ ! -d "MacOS" ] ; then rm -Rf "MacOS" ; mv -f "program" "MacOS" ; ln -sf "MacOS" "program" ; fi'
	mkdir -p "$(INSTALL_HOME)/package/Contents/Library/Spotlight"
	cd "$(INSTALL_HOME)/package/Contents/Library/Spotlight" ; curl -L "$(NEOLIGHT_MDIMPORTER_URL)" | tar zxvf -
	mkdir -p "$(INSTALL_HOME)/package/Contents/Library/QuickLook"
	cd "$(INSTALL_HOME)/package/Contents/Library/QuickLook" ; curl -L "$(NEOPEEK_QLPLUGIN_URL)" | tar zxvf -
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find "." -name ".DS_Store"` ; do rm "$${i}" ; done'
	chmod -Rf a-w,a+r "$(INSTALL_HOME)/package"
	echo "Running sudo to chown installation files..."
	sudo chown -Rf root:admin "$(INSTALL_HOME)/package"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `cd "/Applications/Utilities/Installer.app/Contents/Resources" ; find . -type d -name "*.lproj" -maxdepth 1` ; do mkdir -p "$${i}" ; done'
	printf "pmkrpkg1" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/PkgInfo"
	( cd "$(INSTALL_HOME)/package" ; pax -w -z -x cpio . ) > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Archive.pax.gz"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Info.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Info.plist"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Description.plist" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/Description.plist"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../Description.plist" "$${i}/Description.plist" ; done'
	mkbom "$(INSTALL_HOME)/package" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Archive.bom" >& /dev/null
	cp "etc/gpl.html" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/License.html"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/ReadMe.rtf" | sed 's#$$(PRODUCT_TRADEMARKED_NAME_RTF)#'"$(PRODUCT_TRADEMARKED_NAME_RTF)"'#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/ReadMe.rtf"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(TARGET_FILE_TYPE)#$(TARGET_FILE_TYPE)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/installutils"
	sed 's#$$(TARGET_MACHINE)#$(UNAME)#g' "bin/InstallationCheck" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/InstallationCheck.strings" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck.strings"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../InstallationCheck.strings" "$${i}/InstallationCheck.strings" ; done'
	cp "bin/preflight" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight"
	cp "bin/postflight" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/postflight" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/postflight"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	mv -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	cp -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/ReadMe.rtf" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/ReadMe.rtf"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/src"
	cp -f "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION).src.tar.gz" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/src"
	chmod -Rf a-w,a+r "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	chmod -f u+w "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	mv "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
	sync ; hdiutil create -srcfolder "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)" -format UDZO -ov -o "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).dmg"
	touch "$@"

build.odk_package: build.neo_odk_patches
	touch "$@"

build.patch_package: build.package
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	sh -e -c 'if [ -d "$(PATCH_INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(PATCH_INSTALL_HOME)" ; fi'
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/program/classes"
	chmod -Rf u+w,a+r "$(PATCH_INSTALL_HOME)/package"
	source "$(OO_ENV_JAVA)" ; cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/lib/libvcl$${UPD}$(DLLSUFFIX).dylib" "program"
# With gcc 4.x, we must fully strip the soffice.bin executable
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/bin/soffice" "program/soffice.bin" ; chmod a+x "program/soffice.bin" ; strip "program/soffice.bin"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/share/config"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/class/vcl.jar" "program/classes"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/Library/Preferences/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' > "../../out" ; mv -f "../../out" "program/bootstraprc"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/share/registry/data/org/openoffice/Office"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in "share/registry/data/org/openoffice/Setup.xcu" "share/registry/data/org/openoffice/Office/Common.xcu" ; do cp "$(PWD)/$(INSTALL_HOME)/package/Contents/$${i}" "$${i}" ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share/dict share/registry/modules -type f | grep -v /CVS | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share/dict share/registry/modules -type f | grep -v /CVS` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/class/vcl.jar" "program/classes"
	rm -Rf "$(PATCH_INSTALL_HOME)/package/Contents/Resources"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/Resources"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "$(PWD)/etc/package/Info.plist" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_TRADEMARKED_NAME)#$(PRODUCT_TRADEMARKED_NAME)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g'  | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(PRODUCT_FILETYPE)#$(PRODUCT_FILETYPE)#g' > "Info.plist"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*" -o -name "*.bin"` ; do strip -S -x "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'if [ ! -d "MacOS" ] ; then rm -Rf "MacOS" ; mv -f "program" "MacOS" ; ln -sf "MacOS" "program" ; fi'
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/Library/Spotlight"
	cd "$(PATCH_INSTALL_HOME)/package/Contents/Library/Spotlight" ; curl -L "$(NEOLIGHT_MDIMPORTER_URL)" | tar zxvf -
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/Library/QuickLook"
	cd "$(PATCH_INSTALL_HOME)/package/Contents/Library/QuickLook" ; curl -L "$(NEOPEEK_QLPLUGIN_URL)" | tar zxvf -
	chmod -Rf a-w,a+r "$(PATCH_INSTALL_HOME)/package"
	echo "Running sudo to chown installation files..."
	sudo chown -Rf root:admin "$(PATCH_INSTALL_HOME)/package"
	mkdir -p "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources"
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `cd "/Applications/Utilities/Installer.app/Contents/Resources" ; find . -type d -name "*.lproj" -maxdepth 1` ; do mkdir -p "$${i}" ; done'
	printf "pmkrpkg1" > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/PkgInfo"
	( cd "$(PATCH_INSTALL_HOME)/package" ; pax -w -z -x cpio . ) > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Archive.pax.gz"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Info.plist.patch" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Info.plist"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Description.plist.patch" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/Description.plist"
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../Description.plist" "$${i}/Description.plist" ; done'
	mkbom "$(PATCH_INSTALL_HOME)/package" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Archive.bom" >& /dev/null
	cp "etc/gpl.html" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/License.html"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/ReadMe.rtf" | sed 's#$$(PRODUCT_TRADEMARKED_NAME_RTF)#'"$(PRODUCT_TRADEMARKED_NAME_RTF)"'#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/ReadMe.rtf"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils.patch" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(BUILD_MACHINE)#$(BUILD_MACHINE)#g' | sed 's#$$(TARGET_FILE_TYPE)#$(TARGET_FILE_TYPE)#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/installutils"
	sed 's#$$(TARGET_MACHINE)#$(UNAME)#g' "bin/InstallationCheck" > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/InstallationCheck.strings" > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck.strings"
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../InstallationCheck.strings" "$${i}/InstallationCheck.strings" ; done'
	cp "bin/VolumeCheck.patch" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/VolumeCheck" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/VolumeCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/VolumeCheck.strings.patch" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/VolumeCheck.strings"
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do cp "VolumeCheck.strings" "$${i}/VolumeCheck.strings" ; done' ; rm -f "VolumeCheck.strings"
	cp "bin/preflight.patch" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight"
	cp "bin/postflight.patch" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/postflight" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/postflight"
	source "$(OO_ENV_JAVA)" ; cp "$(BUILD_HOME)/solver/$${UPD}/$(UOUTPUTDIR)/bin/regcomp.bin" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/regcomp.bin" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/regcomp.bin"
	mkdir -p "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	mv -f "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	cp -f "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/ReadMe.rtf" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)/ReadMe.rtf"
	chmod -Rf a-w,a+r "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	chmod -f u+w "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	mv "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
	sync ; hdiutil create -srcfolder "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)" -format UDZO -ov -o "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).dmg"
	touch "$@"

build.package_%: $(INSTALL_HOME)/package_%
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
	chmod -Rf u+w,a+r "$<"
	cd "$</Contents" ; rm -Rf LICENSE* README* licenses/* share/readme/*
	cd "$</Contents" ; cp "$(PWD)/etc/gpl.html" "share/readme/LICENSE_$(PRODUCT_LANG_PACK_LOCALE).html"
	cd "$</Contents" ; cp "$(PWD)/etc/gpl.txt" "share/readme/LICENSE_$(PRODUCT_LANG_PACK_LOCALE)"
	rm -Rf "$</Contents/Resources"
	mkdir -p "$</Contents/Resources"
	cd "$</Contents" ; sh -e -c 'if [ ! -d "MacOS" ] ; then rm -Rf "MacOS" ; mv -f "program" "MacOS" ; ln -sf "MacOS" "program" ; fi'
	cd "$<" ; sh -e -c 'for i in `find "." -name ".DS_Store"` ; do rm "$${i}" ; done'
	chmod -Rf a-w,a+r "$<"
	echo "Running sudo to chown $(@:build.package_%=%) installation files..."
	sudo chown -Rf root:admin "$<"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `cd "/Applications/Utilities/Installer.app/Contents/Resources" ; find . -type d -name "*.lproj" -maxdepth 1` ; do mkdir -p "$${i}" ; done'
	printf "pmkrpkg1" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/PkgInfo"
	( cd "$<" ; pax -w -z -x cpio . ) > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Archive.pax.gz"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Info.plist.langpack" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_LANG_PACK_VERSION)#$(PRODUCT_LANG_PACK_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Info.plist"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Description.plist.langpack" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_LANG_PACK_VERSION)#$(PRODUCT_LANG_PACK_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/Description.plist"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../Description.plist" "$${i}/Description.plist" ; done'
	mkbom "$<" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Archive.bom" >& /dev/null
	cp "etc/gpl.html" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/License.html"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/ReadMe.rtf" | sed 's#$$(PRODUCT_TRADEMARKED_NAME_RTF)#'"$(PRODUCT_TRADEMARKED_NAME_RTF)"'#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/ReadMe.rtf"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils.langpack" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(TARGET_FILE_TYPE)#$(TARGET_FILE_TYPE)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/installutils"
	sed 's#$$(TARGET_MACHINE)#$(UNAME)#g' "bin/InstallationCheck" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/InstallationCheck.strings" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck.strings"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../InstallationCheck.strings" "$${i}/InstallationCheck.strings" ; done'
	cp "bin/VolumeCheck.langpack" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/VolumeCheck" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/VolumeCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/VolumeCheck.strings.langpack" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/VolumeCheck.strings"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do cp "VolumeCheck.strings" "$${i}/VolumeCheck.strings" ; done' ; rm -f "VolumeCheck.strings"
	cp "bin/preflight.langpack" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight"
	cp "bin/postflight.langpack" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/postflight" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/postflight"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)"
	mv -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)"
	cp -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/ReadMe.rtf" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)/ReadMe.rtf"
	chmod -Rf a-w,a+r "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)"
	chmod -f u+w "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)"
	mv "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)"
	sync ; hdiutil create -srcfolder "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME)" -format UDZO -ov -o "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)-$(ULONGNAME).dmg"

build.source_zip:
	rm -Rf "$(SOURCE_HOME)"
	mkdir -p "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	cd "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" ; cvs -d "$(NEO_CVSROOT)" co $(NEO_TAG) "$(NEO_PACKAGE)"
# Prune out empty directories
	cd "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" ; sh -e -c 'for i in `ls -1`; do cd "$${i}" ; cvs update -P -d ; done'
	cp "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/neojava/etc/gpl.html" "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/LICENSE.html"
	chmod -Rf u+w,og-w,a+r "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	cd "$(SOURCE_HOME)" ; gnutar zcf "$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION).src.tar.gz" "$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	touch "$@"

build.cd_package: build.package
	@source "$(OO_ENV_JAVA)" ; sh -c -e 'if [ "$$PRODUCT_NAME" != "$(PRODUCT_NAME)" ] ; then echo "You must rebuild the build.neo_configure target before you can build this target" ; exit 1 ; fi'
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
	touch "$@"

build.all: build.package
	touch "$@"
