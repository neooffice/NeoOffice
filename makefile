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
OO_PATCHES_HOME:=patches/openoffice
OO_ENV_X11:=$(BUILD_HOME)/MacosxEnv.Set
OO_ENV_JAVA:=$(BUILD_HOME)/MacosxEnvJava.Set

# Product information
PRODUCT_NAME=NeoOffice/J(TM)
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
#	cd "$(BUILD_HOME)/config_office" ; autoconf
#	( cd "$(BUILD_HOME)/config_office" ; ./configure CC=cc --with-x )
	rm -f "$(OO_ENV_JAVA)"
	sed 's#^setenv GUIBASE .*$$#setenv GUIBASE "java"#' "$(OO_ENV_X11)" | sed 's#^setenv ENVCDEFS "#&-DUSE_JAVA#' | sed 's#^setenv CLASSPATH .*$$#setenv CLASSPATH "$$SOLARVER/$$UPD/$$INPATH/bin/vcl.jar"#' > "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_NAME '$(PRODUCT_NAME)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_VERSION '$(PRODUCT_VERSION)'" >> "$(OO_ENV_JAVA)"
	echo "setenv PRODUCT_FILETYPE '$(PRODUCT_FILETYPE)'" >> "$(OO_ENV_JAVA)"
#	( cd "$(BUILD_HOME)" ; ./bootstrap )
	touch "$@"

build.oo_all: build.configure
	source "$(OO_ENV_X11)" ; cd "$(BUILD_HOME)/instsetoo" ; `alias build` -all $(BUILD_ARGS)
	touch "$@"

build.neo_%_patch: % build.oo_patches
	cd "$<" ; sh -c 'for i in `cd "$(PWD)/$(BUILD_HOME)/$<" ; find . -type d | grep -v /CVS$$ | grep -v /unxmacxp.pro` ; do mkdir -p "$$i" ; done'
	cd "$<" ; sh -c 'for i in `cd "$(PWD)/$(BUILD_HOME)/$<" ; find . ! -type d | grep -v /CVS/ | grep -v /unxmacxp.pro` ; do if [ ! -f "$$i" ] ; then ln -sf "$(PWD)/$(BUILD_HOME)/$</$$i" "$$i" 2>/dev/null ; fi ; done'
	cd "$<" ; ln -sf "$(PWD)/$(BUILD_HOME)/$</unxmacxp.pro"
	touch "$@"

build.neo_patches: \
	build.neo_dtrans_patch \
	build.neo_forms_patch \
	build.neo_sysui_patch \
	build.neo_toolkit_patch \
	build.neo_vcl_patch
	touch "$@"

build.all: build.oo_all build.neo_patches
	source "$(OO_ENV_JAVA)" ; cd "dtrans" ; `alias build` -u $(BUILD_ARGS)
	source "$(OO_ENV_JAVA)" ; cd "forms" ; `alias build` -u $(BUILD_ARGS)
	source "$(OO_ENV_JAVA)" ; cd "sysui" ; `alias build` -u $(BUILD_ARGS)
	source "$(OO_ENV_JAVA)" ; cd "toolkit" ; `alias build` -u $(BUILD_ARGS)
	source "$(OO_ENV_JAVA)" ; cd "vcl" ; `alias build` -u $(BUILD_ARGS)
	touch "$@"
