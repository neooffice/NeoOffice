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

.IF "$(PRODUCT_DIR_NAME)" != ""
CDEFS += -DPRODUCT_DIR_NAME='"$(PRODUCT_DIR_NAME)"'
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
    $(SLO)$/intro.obj \
    $(SLO)$/langselect.obj \
    $(SLO)$/lockfile.obj \
    $(SLO)$/lockfile2.obj \
    $(SLO)$/migration.obj \
    $(SLO)$/officeipcthread.obj \
    $(SLO)$/oinstanceprovider.obj \
    $(SLO)$/opluginframefactory.obj \
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
    $(OBJ)$/copyright_ascii_sun.obj \
    $(OBJ)$/main.obj

SLOFILES = $(SHL1OBJS)

SRS1NAME=	desktop
SRC1FILES=	desktop.src

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

