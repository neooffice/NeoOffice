#*************************************************************************
#
# Copyright 2008 by Sun Microsystems, Inc.
#
# $RCSfile$
#
# $Revision$
#
# This file is part of NeoOffice.
#
# NeoOffice is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3
# only, as published by the Free Software Foundation.
#
# NeoOffice is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU General Public License
# version 3 along with NeoOffice.  If not, see
# <http://www.gnu.org/licenses/gpl-3.0.txt>
# for a copy of the GPLv3 License.
#
# Modified February 2010 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ = ..$/..$/..

PRJNAME = desktop
TARGET = unopkg
.IF "$(GUI)" == "OS2"
TARGETTYPE = CUI
.ELSE
TARGETTYPE = GUI
.ENDIF
ENABLE_EXCEPTIONS = TRUE
LIBTARGET=NO

PRJINC += ..$/..$/deployment ..$/..
.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/source$/deployment$/inc$/dp_misc.mk

.IF "$(PRODUCT_DIR_NAME)" != ""
CDEFS += -DPRODUCT_DIR_NAME='"$(PRODUCT_DIR_NAME)"'
.ENDIF

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF

APP1TARGET = so$/unopkg
APP1OBJS = $(OBJFILES)
.IF "$(GUIBASE)" == "java"
APP1STDLIBS = -Wl,-rpath,@executable_path/../basis-link/program -Wl,-rpath,@executable_path/../basis-link/ure-link/lib -Wl,-rpath,/usr/lib -Wl,-rpath,/usr/local/lib
.ELSE	# "$(GUIBASE)" == "java"
APP1STDLIBS = $(SALLIB) $(UNOPKGAPPLIB)
.ENDIF		# "$(GUIBASE)" == "java"
APP1DEPN = $(SHL1TARGETN)
APP1NOSAL = TRUE
APP1RPATH = BRAND
.IF "$(OS)" == "WNT"
APP1ICON = $(SOLARRESDIR)$/icons/so9_main_app.ico
APP1LINKRES = $(MISC)$/$(TARGET)1.res
.ENDIF

APP2TARGET = unopkg
APP2OBJS = $(OBJFILES)
.IF "$(GUIBASE)" == "java"
APP2STDLIBS = -Wl,-rpath,@executable_path/../basis-link/program -Wl,-rpath,@executable_path/../basis-link/ure-link/lib -Wl,-rpath,/usr/lib -Wl,-rpath,/usr/local/lib
.ELSE	# "$(GUIBASE)" == "java"
APP2STDLIBS = $(SALLIB) $(UNOPKGAPPLIB)
.ENDIF		# "$(GUIBASE)" == "java"
APP2DEPN = $(SHL1TARGETN)
APP2NOSAL = TRUE
APP2RPATH = BRAND
.IF "$(OS)" == "WNT"
APP2ICON = $(SOLARRESDIR)$/icons/ooo3_main_app.ico
APP2LINKRES = $(MISC)$/$(TARGET)2.res
.ENDIF

SHL1TARGET = unopkgapp
SHL1OBJS = $(SLOFILES) $(SLO)$/lockfile.obj
SHL1STDLIBS = \
    $(SALLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(COMPHELPERLIB) \
    $(UCBHELPERLIB) \
    $(UNOTOOLSLIB) \
    $(TOOLSLIB) \
    $(VCLLIB) \
    $(DEPLOYMENTMISCLIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SLOFILES = \
    $(SLO)$/unopkg_app.obj \
    $(SLO)$/unopkg_cmdenv.obj \
    $(SLO)$/unopkg_misc.obj

OBJFILES = $(OBJ)$/unopkg_main.obj

.IF "$(GUIBASE)" == "java"
OBJFILES += \
    $(OBJ)$/unopkg_java.obj
.ENDIF		# "$(GUIBASE)" == "java"

.INCLUDE : target.mk

.IF "$(APP1TARGETN)" != "" # not set during depend=x
$(APP1TARGETN) : $(MISC)$/binso_created.flg
.ENDIF			# "$(APP1TARGETN)"!=""

$(MISC)$/binso_created.flg:
	@@-$(MKDIRHIER) $(BIN)$/so && $(TOUCH) $@
	@@-$(MKDIRHIER) $(MISC)$/so && $(TOUCH) $@

