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
# Modified December 2005 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=desktop
TARGET=dkt
AUTOSEG=true
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(PRODUCT_NAME)" != "" && "$(PRODUCT_DOMAIN)" != ""

.IF "$(PRODUCT_DIR_NAME)" != ""
CDEFS += -DPRODUCT_DIR_NAME='"$(PRODUCT_DIR_NAME)"'
CDEFS += -DPRODUCT_CHECKSUM="$(shell md5 -q -s '$(PRODUCT_NAME)_$(PRODUCT_DOMAIN).$(PRODUCT_DIR_NAME)')"
.ENDIF

.IF "$(PRODUCT_DIR_NAME2)" != ""
CDEFS += -DPRODUCT_DIR_NAME2='"$(PRODUCT_DIR_NAME2)"'
CDEFS += -DPRODUCT_CHECKSUM2="$(shell md5 -q -s '$(PRODUCT_NAME)_$(PRODUCT_DOMAIN).$(PRODUCT_DIR_NAME2)')"
.ENDIF

.IF "$(PRODUCT_DIR_NAME3)" != ""
CDEFS += -DPRODUCT_DIR_NAME3='"$(PRODUCT_DIR_NAME3)"'
CDEFS += -DPRODUCT_CHECKSUM3="$(shell md5 -q -s '$(PRODUCT_NAME)_$(PRODUCT_DOMAIN).$(PRODUCT_DIR_NAME3)')"
.ENDIF

.ENDIF		# "$(PRODUCT_NAME)" != "" && "$(PRODUCT_DOMAIN)" != ""

.IF "$(GUIBASE)"=="aqua"
CFLAGS+=-x objective-c++
.ENDIF

.IF "$(ENABLE_GNOMEVFS)"=="TRUE"
CFLAGS+=-DGNOME_VFS_ENABLED
.ENDIF

SHL1TARGET = sofficeapp
SHL1OBJS = \
    $(SLO)$/app.obj \
    $(SLO)$/appfirststart.obj \
    $(SLO)$/appinit.obj \
    $(SLO)$/appsys.obj \
    $(SLO)$/cfgfilter.obj \
    $(SLO)$/checkinstall.obj \
    $(SLO)$/cmdlineargs.obj \
    $(SLO)$/cmdlinehelp.obj \
    $(SLO)$/configinit.obj \
    $(SLO)$/desktopcontext.obj \
    $(SLO)$/desktopresid.obj \
    $(SLO)$/dispatchwatcher.obj \
    $(SLO)$/langselect.obj \
    $(SLO)$/lockfile.obj \
    $(SLO)$/lockfile2.obj \
    $(SLO)$/migration.obj \
    $(SLO)$/officeipcthread.obj \
    $(SLO)$/pages.obj \
    $(SLO)$/sofficemain.obj \
    $(SLO)$/userinstall.obj \
    $(SLO)$/wizard.obj
SHL1STDLIBS = \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(I18NISOLANGLIB) \
    $(SALLIB) \
    $(SFXLIB) \
    $(SVLLIB) \
    $(SVTOOLLIB) \
    $(TKLIB) \
    $(TOOLSLIB) \
    $(UCBHELPERLIB) \
    $(UNOTOOLSLIB) \
    $(VCLLIB) \
    $(VOSLIB)
SHL1VERSIONMAP = version.map
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

OBJFILES = \
    $(OBJ)$/copyright_ascii_ooo.obj \
    $(OBJ)$/main.obj
.IF "$(GUI)" != "OS2"
OBJFILES += \
    $(OBJ)$/copyright_ascii_sun.obj
.ENDIF

.IF "$(GUIBASE)" == "java"
OBJFILES += \
    $(OBJ)$/main_java.obj \
    $(OBJ)$/main_java_init.obj \
    $(OBJ)$/main_java_init2.obj \
    $(OBJ)$/main_java_init3.obj
.ENDIF		# "$(GUIBASE)" == "java"

SLOFILES = $(SHL1OBJS)

SRS1NAME=	desktop
SRC1FILES=	desktop.src

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

