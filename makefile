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
INSTALL_HOME:=install
OO_PATCHES_HOME:=patches/openoffice
OO_ENV_X11:=$(BUILD_HOME)/MacosxEnv.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacosxEnvJava.Set

# Product information
PRODUCT_NAME=NeoOffice/J
PRODUCT_DIR_NAME=NeoOfficeJ
# Important: Note that there are escape characters in the PRODUCT_NAME for the
# UTF-8 trademark symbol. Don't replace these with "\x##" literal strings!
PRODUCT_TRADEMARKED_NAME=NeoOffice™/J
PRODUCT_VERSION=0.0
PRODUCT_FILETYPE=no%f

# CVS macros
OO_CVSROOT:=:pserver:anoncvs@anoncvs.openoffice.org:/cvs
OO_PACKAGE:=all
OO_TAG:=OOO_STABLE_1_PORTS

all: build.all

# Create the build directory and checkout the OpenOffice.org sources
build.oo_checkout:
	mkdir -p "$(BUILD_HOME)"
	cd "$(BUILD_HOME)" ; cvs -d "$(OO_CVSROOT)" co -r "$(OO_TAG)" "$(OO_PACKAGE)"
	chmod -Rf u+w "$(BUILD_HOME)"
	touch "$@"

build.oo_patches: build.oo_checkout \
	build.oo_external_patch
	touch "$@"

build.oo_external_patch: build.oo_checkout
	chmod -Rf u+w "$(BUILD_HOME)/external/gpc"
	gnutar zxf "$(OO_PATCHES_HOME)/gpc231.tar.Z" -C "$(BUILD_HOME)/external/gpc"
	chmod -Rf u+w "$(BUILD_HOME)/external/gpc"
	mv -f "$(BUILD_HOME)/external/gpc/gpc231"/* "$(BUILD_HOME)/external/gpc"
	rm -Rf "$(BUILD_HOME)/external/gpc/gpc231"
	-( cd "$(BUILD_HOME)/external" ; patch -R -p0 -N -r "$(PWD)/patch.rej" ) < "$(OO_PATCHES_HOME)/external.patch"
	cp -f "$(OO_PATCHES_HOME)/dlcompat-20020709.tar.gz" "$(BUILD_HOME)/external/download"
	cp -f "$(OO_PATCHES_HOME)/dlcompat.pat.tar.gz" "$(BUILD_HOME)/external/dlcompat"
	( cd "$(BUILD_HOME)/external" ; patch -p0 -N -r "$(PWD)/patch.rej" ) < "$(OO_PATCHES_HOME)/external.patch"
	touch "$@"

build.oo_%_patch: $(BUILD_HOME)/% build.oo_checkout
	-( cd "$<" ; patch -R -p0 -N -r "$(PWD)/patch.rej" ) < "$(OO_PATCHES_HOME)/"`basename "$<"`".patch"
	( cd "$<" ; patch -p0 -N -r "$(PWD)/patch.rej" ) < "$(OO_PATCHES_HOME)/"`basename "$<"`".patch"
	touch "$@"

build.configure: build.oo_patches
	cd "$(BUILD_HOME)/config_office" ; autoconf
	( cd "$(BUILD_HOME)/config_office" ; ./configure CC=cc --with-x )
	rm -f "$(OO_ENV_JAVA)"
	sed 's#^setenv GUIBASE .*$$#setenv GUIBASE "java"#' "$(OO_ENV_X11)" | sed 's#^setenv ENVCDEFS "#&-DUSE_JAVA#' | sed 's#^setenv CLASSPATH .*$$#setenv CLASSPATH "$$SOLARVER/$$UPD/$$INPATH/bin/vcl.jar"#' > "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_NAME '$(PRODUCT_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_DIR_NAME '$(PRODUCT_DIR_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_TRADEMARKED_NAME '$(PRODUCT_TRADEMARKED_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_VERSION '$(PRODUCT_VERSION)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_FILETYPE '$(PRODUCT_FILETYPE)'" >> "$(OO_ENV_JAVA)"
	( cd "$(BUILD_HOME)" ; ./bootstrap )
	touch "$@"

build.oo_all: build.configure
	source "$(OO_ENV_X11)" ; cd "$(BUILD_HOME)/instsetoo" ; `alias build` -all $(BUILD_ARGS)
	touch "$@"

build.neo_%_patch: % build.oo_all
	cd "$<" ; sh -c 'for i in `cd "$(PWD)/$(BUILD_HOME)/$<" ; find . -type d | grep -v /CVS$$ | grep -v /unxmacxp.pro` ; do mkdir -p "$$i" ; done'
	cd "$<" ; sh -c 'for i in `cd "$(PWD)/$(BUILD_HOME)/$<" ; find . ! -type d | grep -v /CVS/ | grep -v /unxmacxp.pro` ; do if [ ! -f "$$i" ] ; then ln -sf "$(PWD)/$(BUILD_HOME)/$</$$i" "$$i" 2>/dev/null ; fi ; done'
	sh -c 'if [ ! -d "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro.oo" -a -d "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro" ] ; then rm -Rf "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro.oo" ; mv -f "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro" "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro.oo" ; fi'
	rm -Rf "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro"
	mkdir -p "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro"
	cd "$<" ; ln -sf "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro"
	source "$(OO_ENV_JAVA)" ; cd "$<" ; `alias build` $(BUILD_ARGS)
	touch "$@"

build.neo_patches: \
	build.neo_desktop_patch \
	build.neo_dtrans_patch \
	build.neo_forms_patch \
	build.neo_readlicense_patch \
	build.neo_offmgr_patch \
	build.neo_sysui_patch \
	build.neo_toolkit_patch \
	build.neo_vcl_patch
	touch "$@"

build.installation: build.neo_patches
	rm -Rf "$(INSTALL_HOME)"
	mkdir -p "$(INSTALL_HOME)"
	echo "[ENVIRONMENT]" > "$(INSTALL_HOME)/response"
	echo "INSTALLATIONMODE=INSTALL_NORMAL" >> "$(INSTALL_HOME)/response"
	echo "INSTALLATIONTYPE=STANDARD" >> "$(INSTALL_HOME)/response"
	echo "DESTINATIONPATH=$(PWD)/$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" >> "$(INSTALL_HOME)/response"
	echo "OUTERPATH=" >> "$(INSTALL_HOME)/response"
	echo "LOGFILE=" >> "$(INSTALL_HOME)/response"
	echo "LANGUAGELIST=<LANGUAGE>" >> "$(INSTALL_HOME)/response"
	echo "[JAVA]" >> "$(INSTALL_HOME)/response"
	echo "JavaSupport=preinstalled_or_none" >> "$(INSTALL_HOME)/response"
	source "$(OO_ENV_JAVA)" ; "$(BUILD_HOME)/instsetoo/unxmacxp.pro/01/normal/setup" -v "-r:$(PWD)/$(INSTALL_HOME)/response"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; cp -f "$(PWD)/$(BUILD_HOME)/dtrans/unxmacxp.pro/lib/libdtransjava$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/forms/unxmacxp.pro/lib/libfrm$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/toolkit/unxmacxp.pro/lib/libtk$${UPD}$${DLLSUFFIX}.dylib" "$(PWD)/$(BUILD_HOME)/vcl/unxmacxp.pro/lib/libvcl$${UPD}$${DLLSUFFIX}.dylib" "program"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; cp -f "$(PWD)/$(BUILD_HOME)/readlicense/source/license/unx/LICENSE" "$(PWD)/$(BUILD_HOME)/readlicense/source/readme/unxmacxp/README" "."
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; cp -f "$(PWD)/$(BUILD_HOME)/offmgr/unxmacxp.pro/bin/neojava$${UPD}01.res" "program/resource/iso$${UPD}01.res"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; cp -f "$(PWD)/$(BUILD_HOME)/sysui/unxmacxp.pro/misc/nswrapper.sh" "program/nswrapper"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; cp -f "$(PWD)/$(BUILD_HOME)/sysui/unxmacxp.pro/misc/soffice.sh" "program/soffice"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; cp -f "$(PWD)/$(BUILD_HOME)/sysui/unxmacxp.pro/misc/Info.plist" "."
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; cp -f "$(PWD)/$(BUILD_HOME)/sysui/unxmacxp.pro/misc/PkgInfo" "."
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; cp -f "$(PWD)/$(BUILD_HOME)/vcl/unxmacxp.pro/class/vcl.jar" "program/classes"
	rm -Rf "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents/Resources"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents/Resources"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; cp -f "$(PWD)/$(BUILD_HOME)/sysui/unxmacxp.pro/misc/ship.icns" "Resources"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents/program" ; regcomp -revoke -r applicat.rdb -c "libdtransX11$${UPD}$${DLLSUFFIX}.dylib"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents/program" ; regcomp -register -r applicat.rdb -c "libdtransjava$${UPD}$${DLLSUFFIX}.dylib"
	source "$(OO_ENV_JAVA)" ; cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; rm -Rf "license.html" "readme.html" "setup" "spadmin" "program/libdtransX11$${UPD}$${DLLSUFFIX}.dylib" "program/setup" "program/setup.bin" "program/spadmin" "program/spadmin.bin" "share/config/registry/cache" "user"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; sed 's#ProductPatch=.*$$#ProductPatch=($(PRODUCT_VERSION))#' "program/bootstraprc" | sed 's#Location=.*$$#Location=$$SYSUSERCONFIG/.neojavarc#' | sed 's#ProductKey=.*$$#ProductKey=$(PRODUCT_NAME) $(PRODUCT_VERSION)#' > "../../out" ; mv -f "../../out" "program/bootstraprc"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; sed 's#"string">.*</ooName>#"string">$(PRODUCT_NAME)</ooName>#g' "share/config/registry/instance/org/openoffice/Setup.xml" | sed 's#"string">.*</ooSetupVersion>#"string">$(PRODUCT_VERSION)</ooSetupVersion>#g'> "../../out" ; mv -f "../../out" "share/config/registry/instance/org/openoffice/Setup.xml"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; sed 's#"string">.*</OfficeInstall>#"string">/Applications/$(PRODUCT_DIR_NAME).app/Contents</OfficeInstall>#g' "share/config/registry/instance/org/openoffice/Office/Common.xml" | sed 's#>OpenOffice\.org [0-9\.]* #>$(PRODUCT_NAME) $(PRODUCT_VERSION) #g'> "../../out" ; mv -f "../../out" "share/config/registry/instance/org/openoffice/Setup.xml"
	cd "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app/Contents" ; sh -c 'if [ ! -d "MacOS" ] ; then rm -Rf "MacOS" ; mv -f "program" "MacOS" ; ln -s "MacOS" "program" ; fi'
	chmod -Rf u+w,go-w,a+r "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME).app"
	touch $@

# This target must be run manually since it launches the GUI PackageMaker tool
build.package: build.installation
	rm -Rf "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)"
	mkdir -p "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)"
	cp -f etc/gpl.html "$(INSTALL_HOME)/$(PRODUCT_DIR_NAME)/License.html"
	rm -f "$(INSTALL_HOME)/neojava.pmsp"
	sed 's#$$(INSTALL_HOME)#$(PWD)/$(INSTALL_HOME)#g' etc/neojava.pmsp | sed 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' | sed 's#$$(PRODUCT_DIR_NAME)#$(PRODUCT_DIR_NAME)#g' | sed 's#$$(PRODUCT_VERSION)#$(PRODUCT_VERSION)#g' > "$(INSTALL_HOME)/neojava.pmsp"
	open "$(PWD)/$(INSTALL_HOME)/neojava.pmsp"

build.all: build.oo_all build.installation
	touch "$@"
