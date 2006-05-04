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

# Macros that are overridable by make command line options
CC=cc
CXX=c++
GNUCP=/opt/local/bin/gcp
PKG_CONFIG=/opt/local/bin/pkg-config

# Set the shell to tcsh since the OpenOffice.org build requires it
SHELL:=/bin/tcsh
UNAME:=$(shell uname -p)
ifeq ("$(UNAME)","powerpc")
ULONGNAME=PowerPC
UOUTPUTDIR=unxmacxp.pro
else
ULONGNAME=Intel
UOUTPUTDIR=unxmacxi.pro
endif

# Build location macros
BUILD_HOME:=build
INSTALL_HOME:=install
PATCH_INSTALL_HOME:=patch_install
SOURCE_HOME:=source
CD_INSTALL_HOME:=cd_install
OO_PATCHES_HOME:=patches/openoffice
ifeq ("$(UNAME)","powerpc")
OO_ENV_X11:=$(BUILD_HOME)/MacOSXPPCEnv.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacOSXPPCEnvJava.Set
else
OO_ENV_X11:=$(BUILD_HOME)/MacOSXIntelEnv.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacOSXIntelEnvJava.Set
endif
OO_LANGUAGES=ALL
NEOLIGHT_MDIMPORTER_URL:=http://trinity.neooffice.org/downloads/neolight.mdimporter.tgz

# Product information
OO_PRODUCT_NAME=OpenOffice.org
OO_PRODUCT_VERSION=2.0
OO_DIR_NAME=openoffice.org2.0
OO_REGISTRATION_URL=http://www.openoffice.org/welcome/registration20.html
OO_SUPPORT_URL=http://www.openoffice.org
OO_SUPPORT_URL_TEXT=www.openoffice.org
PRODUCT_NAME=NeoOffice
PRODUCT_DIR_NAME=NeoOffice
# Important: Note that there are escape characters in the PRODUCT_NAME for the
# UTF-8 trademark symbol. Don't replace these with "\x##" literal strings!
PRODUCT_TRADEMARKED_NAME=NeoOffice®
PRODUCT_TRADEMARKED_NAME_RTF=NeoOffice\\\'a8
PRODUCT_VERSION_FAMILY=2.x
ifeq ("$(UNAME)","powerpc")
PRODUCT_VERSION=2.0 Alpha
PRODUCT_DIR_VERSION=2.0_Alpha
else
PRODUCT_VERSION=2.0 Alpha
PRODUCT_DIR_VERSION=2.0_Alpha
endif
PRODUCT_LANG_PACK_VERSION=Language Pack
PRODUCT_DIR_LANG_PACK_VERSION=Language_Pack
ifeq ("$(UNAME)","powerpc")
PRODUCT_PATCH_VERSION=Patch 2
PRODUCT_DIR_PATCH_VERSION=Patch-2
else
PRODUCT_PATCH_VERSION=
PRODUCT_DIR_PATCH_VERSION=
endif
# Don't allow patching of pre-2.0 installations
PRODUCT_PREVIOUS_VERSION=1.9
PRODUCT_PREVIOUS_PATCH_VERSION=99
PRODUCT_FILETYPE=NO%F
PRODUCT_INSTALL_URL=http://www.planamesa.com/neojava/download.php\\\#install
PRODUCT_BUILD_URL=http://www.planamesa.com/neojava/build.php
PRODUCT_PATCH_DOWNLOAD_URL=http://www.planamesa.com/neojava/patch.php
PRODUCT_REGISTRATION_URL=http://trinity.neooffice.org/modules.php?name=Your_Account\&amp\;redirect=index
PRODUCT_DONATION_URL=http://www.planamesa.com/neojava/donate.php
PRODUCT_SUPPORT_URL=http://trinity.neooffice.org/modules.php?name=Forums
PRODUCT_SUPPORT_URL_TEXT:=$(PRODUCT_NAME) Support

# CVS macros
OO_CVSROOT:=:pserver:anoncvs@anoncvs.services.openoffice.org:/cvs
OO_PACKAGES:=OpenOffice2
OO_TAG:=OpenOffice_2_0_2
OO_SOURCE_TAR_GZ_FILE:=$(PWD)/OOo_2.0.2_src.tar.gz
OO_SOURCE_OUTPUT_DIR:=OOB680_m5
NEO_CVSROOT:=:pserver:anoncvs@anoncvs.neooffice.org:/cvs
NEO_PACKAGE:=NeoOffice
NEO_TAG:=NeoOffice-2_0_Alpha

all: build.all

# Create the build directory and checkout the OpenOffice.org sources
build.oo_checkout:
	mkdir -p "$(BUILD_HOME)"
# The OOo cvs server gets messed up with tags so we need to do a little trick
# to get the checkout to work
	sh -e -c 'if [ ! -e "$(OO_SOURCE_TAR_GZ_FILE)" ] ; then rm -Rf "$(BUILD_HOME)/tmp" ; mkdir -p "$(BUILD_HOME)/tmp" ; cd "$(BUILD_HOME)/tmp" ; cvs -d "$(OO_CVSROOT)" co MathMLDTD ; cd MathMLDTD ; cvs update -d -r "$(OO_TAG)" ; fi'
	rm -Rf "$(BUILD_HOME)/tmp"
# Do the real checkout
	sh -e -c 'if [ -e "$(OO_SOURCE_TAR_GZ_FILE)" ] ; then pax -z -v -r -s "#$(OO_SOURCE_OUTPUT_DIR)#$(BUILD_HOME)#" -f "$(OO_SOURCE_TAR_GZ_FILE)" ; fi'
	-sh -e -c 'if [ ! -e "$(OO_SOURCE_TAR_GZ_FILE)" ] ; then cd "$(BUILD_HOME)" ; cvs -d "$(OO_CVSROOT)" co -r "$(OO_TAG)" $(OO_PACKAGES) ; fi'
# cvs seems to always fail so check that the last module has been checked out
	cd "$(BUILD_HOME)/agg" ; cvs -d "$(OO_CVSROOT)" update -r "$(OO_TAG)"
	sh -e -c 'if [ ! -e "$(OO_SOURCE_TAR_GZ_FILE)" ] ; then cd "$(BUILD_HOME)/agg" ; cvs -d "$(OO_CVSROOT)" update -r "$(OO_TAG)" ; fi'
	chmod -Rf u+w "$(BUILD_HOME)"
	touch "$@"

build.oo_patches: build.oo_checkout \
	build.oo_automation_patch \
	build.oo_berkeleydb_patch \
	build.oo_binfilter_patch \
	build.oo_extensions_patch \
	build.oo_external_patch \
	build.oo_forms_patch \
	build.oo_instsetoo_native_patch \
	build.oo_jvmfwk_patch \
	build.oo_padmin_patch \
	build.oo_scp2_patch \
	build.oo_sj2_patch \
	build.oo_solenv_patch \
	build.oo_toolkit_patch \
	build.oo_xmerge_patch
	touch "$@"

build.oo_odk_patches: build.oo_checkout
	touch "$@"

build.oo_external_patch: build.oo_checkout
	chmod -Rf u+w "$(BUILD_HOME)/external/gpc"
	gnutar zxf "$(OO_PATCHES_HOME)/gpc231.tar.Z" -C "$(BUILD_HOME)/external/gpc"
	chmod -Rf u+w "$(BUILD_HOME)/external/gpc"
	mv -f "$(BUILD_HOME)/external/gpc/gpc231"/* "$(BUILD_HOME)/external/gpc"
	rm -Rf "$(BUILD_HOME)/external/gpc/gpc231"
	touch "$@"

build.oo_%_patch: $(OO_PATCHES_HOME)/%.patch build.oo_checkout
	-( cd "$(BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -R -p0 -N -r "/dev/null" ) < "$<"
	( cd "$(BUILD_HOME)/$(@:build.oo_%_patch=%)" ; patch -p0 -N -r "$(PWD)/patch.rej" ) < "$<"
	touch "$@"

build.configure: build.oo_patches
	cd "$(BUILD_HOME)/config_office" ; autoconf
	( cd "$(BUILD_HOME)/config_office" ; setenv PATH "/bin:/sbin:/usr/bin:/usr/sbin" ; unsetenv DYLD_LIBRARY_PATH ; ./configure CC=$(CC) CXX=$(CXX) PKG_CONFIG=$(PKG_CONFIG) --with-jdk-home=/System/Library/Frameworks/JavaVM.framework/Home --with-epm=internal --disable-gtk --disable-mozilla --without-nas --with-gnu-cp="$(GNUCP)" --with-system-curl --with-x --x-includes=/usr/X11R6/include --with-lang="$(OO_LANGUAGES)" )
	echo "unsetenv LD_SEG_ADDR_TABLE" >> "$(OO_ENV_X11)"
	echo "unsetenv LD_PREBIND" >> "$(OO_ENV_X11)"
	echo "unsetenv LD_PREBIND_ALLOW_OVERLAP" >> "$(OO_ENV_X11)"
	( cd "$(BUILD_HOME)" ; ./bootstrap )
	touch "$@"

build.oo_all: build.configure
	source "$(OO_ENV_X11)" ; cd "$(BUILD_HOME)/instsetoo_native" ; `alias build` --all $(OO_BUILD_ARGS)
	touch "$@"

build.oo_odk_all: build.configure build.oo_all build.oo_odk_patches
	source "$(OO_ENV_X11)" ; cd "$(BUILD_HOME)/odk" ; `alias build` $(OO_BUILD_ARGS)
	touch "$@"

build.neo_configure: build.oo_all
	rm -f "$(OO_ENV_JAVA)"
	sed 's#^setenv GUIBASE .*$$#setenv GUIBASE "java"#' "$(OO_ENV_X11)" | sed 's#^setenv ENVCDEFS "#&-DUSE_JAVA#' | sed 's#^setenv CLASSPATH .*$$#setenv CLASSPATH "$$SOLARVER/$$UPD/$$INPATH/bin/vcl.jar"#' | sed 's#^setenv DELIVER .*$$#setenv DELIVER "true"#' | sed 's#^alias deliver .*$$#alias deliver "echo The deliver command has been disabled"#' > "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_NAME '$(PRODUCT_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_DIR_NAME '$(PRODUCT_DIR_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_DONATION_URL '$(PRODUCT_DONATION_URL)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_TRADEMARKED_NAME '$(PRODUCT_TRADEMARKED_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_VERSION '$(PRODUCT_VERSION)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_FILETYPE '$(PRODUCT_FILETYPE)'" >> "$(OO_ENV_JAVA)"
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

build.neo_patches: build.oo_all \
	build.neo_canvas_patch \
	build.neo_desktop_patch \
	build.neo_dtrans_patch \
	build.neo_framework_patch \
	build.neo_jvmfwk_patch \
	build.neo_sal_patch \
	build.neo_sfx2_patch \
	build.neo_shell_patch \
	build.neo_sj2_patch \
	build.neo_store_patch \
	build.neo_svtools_patch \
	build.neo_svx_patch \
	build.neo_sw_patch \
	build.neo_sysui_patch \
	build.neo_vcl_patch
	touch "$@"

build.neo_odk_patches: \
	build.oo_odk_all \
	build.neo_odk_patch
	touch "$@"

build.package: build.neo_patches build.source_zip
	sh -e -c 'if [ -d "$(INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(INSTALL_HOME)" ; fi'
	mkdir -p "$(INSTALL_HOME)/package/Contents"
	mkdir -p "$(INSTALL_HOME)/package/Applications"
	ln -sf "$(PWD)/$(INSTALL_HOME)/package/Contents" "$(PWD)/$(INSTALL_HOME)/package/Applications/$(OO_DIR_NAME)"
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find "$(PWD)/$(BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice/portable/install" -name "*.sw"` ; do gnutar xvf "$${i}" ; done'
# Regroup the OOo langauge packs
	cd "$(INSTALL_HOME)/package" ; sh -e -c 'for i in `find "$(PWD)/$(BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice/portable/install" -name "*.install"` ; do grep "mkdir " "$${i}" | grep "/Applications" | sed "s#^.*/Applications#Applications#" | xargs -n1 mkdir -p ; done'
	rm -Rf "$(INSTALL_HOME)/package/Applications"
	cd "$(BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/portable/install" ; find . -type d -maxdepth 1 -exec basename {} \; | grep -v '^\.$$' | grep -v '^log$$' > "$(PWD)/$(INSTALL_HOME)/language_names"
# Create the language pack installers
	source "$(OO_ENV_JAVA)" ; sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names"` ; do if [ "$${i}" = "en-US" ] ; then continue ; fi ; langname=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | awk -F, "{ print \\$$3 }"` ; langdirname=`echo "$${langname}" | sed "s# #_#g"` ; if [ -z "$${langname}" -o -z "$${langdirname}" ] ; then echo "Skipping $${i} language..." ; continue ; fi ; mkdir -p "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents" ; mkdir -p "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Applications" ; ln -sf "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents" "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Applications/$(OO_DIR_NAME)" ; ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}" ; for j in `find "$(PWD)/$(BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/portable/install/$${i}" -name "*.sw"` ; do gnutar xvf "$${j}" ; done ; for j in `find "$(PWD)/$(BUILD_HOME)/instsetoo_native/$(UOUTPUTDIR)/OpenOffice_languagepack/portable/install/$${i}" -name "*.install"` ; do grep "mkdir " "$${j}" | grep "/Applications" | sed "s#^.*/Applications#Applications#" | xargs -n1 mkdir -p ; done ) ; rm -Rf "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Applications" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/bin/salapp$${UPD}$${i}.res" "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/program/resource" ; helpflag=`grep "^$${i}," "$(PWD)/etc/supportedlanguages.txt" | awk -F, "{ print \\$$2 }"` ; if [ "$${helpflag}" != "1" ] ; then rm -Rf "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help/$${i}" ; ( cd "$(PWD)/$(INSTALL_HOME)/package_$${langdirname}/Contents/help" ; ln -s "en" "$${i}" ) ; fi ; "$(MAKE)" $(MFLAGS) "PRODUCT_LANG_PACK_LOCALE=$${i}" "PRODUCT_LANG_PACK_VERSION=$(PRODUCT_LANG_PACK_VERSION) $${langname}" "PRODUCT_DIR_LANG_PACK_VERSION=$(PRODUCT_DIR_LANG_PACK_VERSION)_$${langdirname}" "build.package_$${langdirname}" ; done'
	chmod -Rf u+w,a+r "$(INSTALL_HOME)/package"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/canvas/$(UOUTPUTDIR)/lib/vclcanvas.uno.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/deployment$${UPD}$${DLLSUFFIX}.uno.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/deploymentgui$${UPD}$${DLLSUFFIX}.uno.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/libspl$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/dtrans/$(UOUTPUTDIR)/lib/libdtransjava$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/framework/$(UOUTPUTDIR)/lib/libfwk$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/bin/sunjavapluginrc" "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/lib/libjvmfwk.dylib.3" "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/lib/sunjavaplugin.dylib" "$(PWD)/$(BUILD_HOME)/sal/$(UOUTPUTDIR)/lib/libsalextra_x11osx_$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/sal/$(UOUTPUTDIR)/lib/libuno_sal.dylib.3" "$(PWD)/$(BUILD_HOME)/sfx2/$(UOUTPUTDIR)/lib/libsfx$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/shell/$(UOUTPUTDIR)/lib/librecentfile.dylib" "$(PWD)/$(BUILD_HOME)/shell/$(UOUTPUTDIR)/lib/localebe1.uno.dylib" "$(PWD)/$(BUILD_HOME)/sj2/$(UOUTPUTDIR)/lib/libj$${UPD}$${DLLSUFFIX}_g.dylib" "$(PWD)/$(BUILD_HOME)/svtools/$(UOUTPUTDIR)/lib/libsvl$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/svtools/$(UOUTPUTDIR)/lib/libsvt$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/store/$(UOUTPUTDIR)/lib/libstore.dylib.3" "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/lib/libsvx$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/sw/$(UOUTPUTDIR)/lib/libsw$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/sw/$(UOUTPUTDIR)/lib/libswui$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/lib/libvcl$${UPD}$${DLLSUFFIX}.dylib" "program"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/bin/salapp$${UPD}en-US.res" "program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/bin/pkgchk" "program/pkgchk.bin" ; chmod a+x "program/pkgchk.bin"
# With gcc 4.x, we must fully strip the soffice.bin executable
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/bin/soffice" "program/soffice.bin" ; chmod a+x "program/soffice.bin" ; strip "program/soffice.bin"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents" ; rm -f "program/resource/ooo$${UPD}"*.res ; cp -f "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/bin/ooo$${UPD}"*.res "program/resource"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/shell/$(UOUTPUTDIR)/bin/senddoc" "program/senddoc" ; chmod a+x "program/senddoc"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/bin/javavendors_ooo.xml" "share/config/javavendors.xml"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/sysui/$(UOUTPUTDIR)/misc/Info.plist" "."
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/sysui/$(UOUTPUTDIR)/misc/PkgInfo" "."
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/class/vcl.jar" "program/classes"
	rm -Rf "$(INSTALL_HOME)/package/Contents/Resources"
	mkdir -p "$(INSTALL_HOME)/package/Contents/Resources"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/sysui/$(UOUTPUTDIR)/misc/License" "$(PWD)/$(BUILD_HOME)/sysui/$(UOUTPUTDIR)/misc/Readme" "$(PWD)/$(BUILD_HOME)/sysui/$(UOUTPUTDIR)/misc/"*.icns "Resources"
	cd "$(INSTALL_HOME)/package/Contents/Resources" ; sh -e -c 'for i in `cat "$(PWD)/$(INSTALL_HOME)/language_names" | sed "s#-#_#g"` ; do mkdir -p "$${i}.lproj" ; mkdir -p `echo "$${i}" | sed "s#_.*\\$$##"`".lproj" ; done'
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/program" ; regcomp -revoke -r services.rdb -c "libdtransX11$${UPD}$${DLLSUFFIX}.dylib"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/program" ; regcomp -register -r services.rdb -c "libdtransjava$${UPD}$${DLLSUFFIX}.dylib"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents/program" ; regcomp -register -r services.rdb -c "libwpft$${UPD}$${DLLSUFFIX}.dylib"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/package/Contents" ; rm -Rf LICENSE* README* licenses/* "program/cde-open-url" "program/configimport" "program/gengal" "program/gnome-open-url" "program/gnome-open-url.bin" "program/kde-open-url" "program/libdtransX11$${UPD}$${DLLSUFFIX}.dylib" "program/pluginapp.bin" "program/libpsp$${UPD}$${DLLSUFFIX}.dylib" "program/libspa$${UPD}$${DLLSUFFIX}.dylib" "program/libvclplug_gen$${UPD}$${DLLSUFFIX}.dylib" "program/open-url" "program/setofficelang" "program/oo_product.bmp" "program/soffice" "program/spadmin" "program/spadmin.bin" "program/unopkg" readmes/* "share/psprint" share/readme/*
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/gpl.html" "share/readme/LICENSE_en-US.html"
	cd "$(INSTALL_HOME)/package/Contents" ; cp "$(PWD)/etc/gpl.txt" "share/readme/LICENSE_en-US"
	cd "$(INSTALL_HOME)/package/Contents/program" ; ln -sf "configimport.bin" "configimport"
	cd "$(INSTALL_HOME)/package/Contents/program" ; ln -sf "gengal.bin" "gengal"
	cd "$(INSTALL_HOME)/package/Contents/program" ; ln -sf "pkgchk.bin" "pkgchk"
	cd "$(INSTALL_HOME)/package/Contents/program" ; ln -sf "setofficelang.bin" "setofficelang"
	cd "$(INSTALL_HOME)/package/Contents/program" ; ln -sf "soffice.bin" "soffice"
	cd "$(INSTALL_HOME)/package/Contents/program" ; ln -sf "unopkg.bin" "unopkg"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find program share -type f | grep -v /CVS | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find program share -type f | grep -v /CVS` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/Library/Preferences/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' > "../../out" ; mv -f "../../out" "program/bootstraprc"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in "share/registry/data/org/openoffice/Setup.xcu" "share/registry/data/org/openoffice/Office/Common.xcu" ; do sed "s#>$(OO_PRODUCT_NAME)<#>$(PRODUCT_NAME)<#g" "$${i}" | sed "s#>$(OO_PRODUCT_VERSION)<#>$(PRODUCT_VERSION)<#g" | sed "s#>$(OO_REGISTRATION_URL)<#>$(PRODUCT_REGISTRATION_URL)<#g" > "../../out" ; mv -f "../../out" "$${i}" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sed 's#$$(OO_PRODUCT_NAME)#$(OO_PRODUCT_NAME)#g' "$(PWD)/etc/help/main_transform.xsl" | sed 's#$$(OO_SUPPORT_URL)#$(OO_SUPPORT_URL)#g' | sed 's#$$(OO_SUPPORT_URL_TEXT)#$(OO_SUPPORT_URL_TEXT)#g' | sed 's#$$(PRODUCT_SUPPORT_URL)#$(PRODUCT_SUPPORT_URL)#g' | sed 's#$$(PRODUCT_SUPPORT_URL_TEXT)#$(PRODUCT_SUPPORT_URL_TEXT)#g' > "help/main_transform.xsl"
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*" -o -name "*.bin"` ; do strip -S -x "$$i" ; done'
	cd "$(INSTALL_HOME)/package/Contents" ; sh -e -c 'if [ ! -d "MacOS" ] ; then rm -Rf "MacOS" ; mv -f "program" "MacOS" ; ln -sf "MacOS" "program" ; fi'
	mkdir -p "$(INSTALL_HOME)/package/Contents/Library/Spotlight"
	cd "$(INSTALL_HOME)/package/Contents/Library/Spotlight" ; curl -L "$(NEOLIGHT_MDIMPORTER_URL)" | tar zxvf -
	cd "$(INSTALL_HOME)" ; curl -L -O "$(NEOLIGHT_MDIMPORTER_URL)"
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
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/installutils"
	sed 's#$$(TARGET_MACHINE)#$(UNAME)#g' "bin/InstallationCheck" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/InstallationCheck.strings" > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck.strings"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../InstallationCheck.strings" "$${i}/InstallationCheck.strings" ; done'
	cp "bin/preflight" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight"
	sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' "bin/postflight" | sed 's#$$(PRODUCT_DIR_VERSION)#$(PRODUCT_DIR_VERSION)#g' | sed 's#$$(ULONGNAME)#$(ULONGNAME)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/postflight" ; chmod a+x "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/postflight"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	mv -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	cp -f "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/ReadMe.rtf" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/ReadMe.rtf"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/src"
	cp -f "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION).src.tar.gz" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/src"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "etc/Build.html" | sed 's#$$(PRODUCT_BUILD_URL)#$(PRODUCT_BUILD_URL)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/src/Build.html"
	chmod -Rf a-w,a+r "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	chmod -f u+w "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	mv "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
	sync ; hdiutil create -srcfolder "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)" -format UDZO -ov -o "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).dmg"
	touch "$@"

build.odk_package: build.neo_odk_patches
	touch "$@"

build.patch_package: build.package
	sh -e -c 'if [ -d "$(PATCH_INSTALL_HOME)" ] ; then echo "Running sudo to delete previous installation files..." ; sudo rm -Rf "$(PWD)/$(PATCH_INSTALL_HOME)" ; fi'
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/program/classes"
	chmod -Rf u+w,a+r "$(PATCH_INSTALL_HOME)/package"
ifeq ("$(UNAME)","powerpc")
	source "$(OO_ENV_JAVA)" ; cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/deployment$${UPD}$${DLLSUFFIX}.uno.dylib" "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/lib/deploymentgui$${UPD}$${DLLSUFFIX}.uno.dylib" "$(PWD)/$(BUILD_HOME)/framework/$(UOUTPUTDIR)/lib/libfwk$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/jvmfwk/$(UOUTPUTDIR)/lib/libjvmfwk.dylib.3" "$(PWD)/$(BUILD_HOME)/sal/$(UOUTPUTDIR)/lib/libuno_sal.dylib.3" "$(PWD)/$(BUILD_HOME)/sfx2/$(UOUTPUTDIR)/lib/libsfx$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/svx/$(UOUTPUTDIR)/lib/libsvx$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/lib/libvcl$${UPD}$${DLLSUFFIX}.dylib" "program"
# With gcc 4.x, we must fully strip the soffice.bin executable
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/desktop/$(UOUTPUTDIR)/bin/soffice" "program/soffice.bin" ; chmod a+x "program/soffice.bin" ; strip "program/soffice.bin"
else
	source "$(OO_ENV_JAVA)" ; cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/lib/libvcl$${UPD}$${DLLSUFFIX}.dylib" "program"
endif
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/sysui/$(UOUTPUTDIR)/misc/Info.plist" "."
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/class/vcl.jar" "program/classes"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sed '/Location=.*$$/d' "$(PWD)/etc/program/bootstraprc" | sed 's#UserInstallation=.*$$#UserInstallation=$$SYSUSERCONFIG/Library/Preferences/$(PRODUCT_DIR_NAME)-$(PRODUCT_VERSION_FAMILY)#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' | sed 's#ProductPatch=.*$$#ProductPatch=$(PRODUCT_PATCH_VERSION)#' > "../../out" ; mv -f "../../out" "program/bootstraprc"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share/basic -type f | grep -v /CVS | xargs -n1 dirname` ; do mkdir -p $${i} ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `cd "$(PWD)/etc" ; find share/basic -type f | grep -v /CVS` ; do cp "$(PWD)/etc/$${i}" "$${i}" ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; cp "$(PWD)/$(BUILD_HOME)/vcl/$(UOUTPUTDIR)/class/vcl.jar" "program/classes"
	rm -Rf "$(PATCH_INSTALL_HOME)/package/Contents/Resources"
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/Resources"
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'for i in `find . -type f -name "*.dylib*" -o -name "*.bin"` ; do strip -S -x "$$i" ; done'
	cd "$(PATCH_INSTALL_HOME)/package/Contents" ; sh -e -c 'if [ ! -d "MacOS" ] ; then rm -Rf "MacOS" ; mv -f "program" "MacOS" ; ln -sf "MacOS" "program" ; fi'
	mkdir -p "$(PATCH_INSTALL_HOME)/package/Contents/Library/Spotlight"
	cd "$(PATCH_INSTALL_HOME)/package/Contents/Library/Spotlight" ; curl -L "$(NEOLIGHT_MDIMPORTER_URL)" | tar zxvf -
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
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils.patch" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' | sed 's#$$(PRODUCT_PREVIOUS_VERSION)#$(PRODUCT_PREVIOUS_VERSION)#g' | sed 's#$$(PRODUCT_PREVIOUS_PATCH_VERSION)#$(PRODUCT_PREVIOUS_PATCH_VERSION)#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/installutils"
	sed 's#$$(TARGET_MACHINE)#$(UNAME)#g' "bin/InstallationCheck" > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/InstallationCheck.strings" > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/InstallationCheck.strings"
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do ln -sf "../InstallationCheck.strings" "$${i}/InstallationCheck.strings" ; done'
	cp "bin/VolumeCheck.patch" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/VolumeCheck" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/VolumeCheck"
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/VolumeCheck.strings.patch" | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/VolumeCheck.strings"
	cd "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources" ; sh -e -c 'for i in `find . -type d -name "*.lproj"` ; do cp "VolumeCheck.strings" "$${i}/VolumeCheck.strings" ; done' ; rm -f "VolumeCheck.strings"
	cp "bin/preflight.patch" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/preflight"
	cp "bin/postflight.patch" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/postflight" ; chmod a+x "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/postflight"
	mkdir -p "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	mv -f "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	cp -f "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/ReadMe.rtf" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)/ReadMe.rtf"
	chmod -Rf a-w,a+r "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	chmod -f u+w "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)"
	mv "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)" "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)"
	sync ; hdiutil create -srcfolder "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME)" -format UDZO -ov -o "$(PATCH_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_PATCH_VERSION)-$(ULONGNAME).dmg"
	touch "$@"

build.package_%: $(INSTALL_HOME)/package_%
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
	sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' "bin/installutils.langpack" | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION_FAMILY)#$(PRODUCT_VERSION_FAMILY)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' | sed 's#$$(PRODUCT_PATCH_VERSION)#$(PRODUCT_PATCH_VERSION)#g' > "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).pkg/Contents/Resources/installutils"
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
	cd "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" ; cvs -d "$(NEO_CVSROOT)" co -r "$(NEO_TAG)" "$(NEO_PACKAGE)"
# Prune out empty directories
	cd "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" ; sh -e -c 'for i in `ls -1`; do cd "$${i}" ; cvs update -P -d ; done'
	cp "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/neojava/etc/gpl.html" "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/LICENSE.html"
	chmod -Rf u+w,og-w,a+r "$(SOURCE_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	cd "$(SOURCE_HOME)" ; gnutar zcf "$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION).src.tar.gz" "$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	touch "$@"

build.cd_package: build.package
	sh -e -c 'if [ -d "$(CD_INSTALL_HOME)" ] ; then chmod -Rf a+rw "$(CD_INSTALL_HOME)" ; fi'
	rm -Rf "$(CD_INSTALL_HOME)"
	mkdir -p "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/Language Packs"
	cd "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)/Language Packs" ; sh -e -c 'for i in `cd "$(PWD)/$(INSTALL_HOME)" ; find . -type d -name "$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(PRODUCT_DIR_LANG_PACK_VERSION)_*" -maxdepth 1` ; do ( cd "$(PWD)/$(INSTALL_HOME)" ; tar cf - "$$i" ) | tar xf - ; done'
	cd "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" ; ( cd "$(PWD)/$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)" ; tar cf - * ) | tar xf -
	chmod -Rf a-w,a+r "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	chmod -f u+w "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)"
	mv "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)" "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)"
	sync ; hdiutil create -srcfolder "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME)" -format UDTO -ov -o "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).cdr.dmg"
	mv "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).cdr.dmg.cdr" "$(CD_INSTALL_HOME)/$(PRODUCT_DIR_NAME)-$(PRODUCT_DIR_VERSION)-$(ULONGNAME).cdr.dmg"
	touch "$@"

build.all: build.package
	touch "$@"
