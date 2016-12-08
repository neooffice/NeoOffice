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
# Modified December 2016 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..
PRJINC=$(PRJ)/inc
PRJNAME=svtools
TARGET=udlg
USE_DEFFILE=TRUE

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk

.IF "$(UPD)" == "310"
PREPENDLIBS=$(PRJ)$/..$/comphelper$/$(INPATH)$/lib

# Link to modified libraries
SOLARLIB:=-L$(PREPENDLIBS) $(SOLARLIB)
SOLARLIBDIR:=$(PREPENDLIBS) -L$(SOLARLIBDIR)
.ENDIF		# "$(UPD)" == "310"

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =	\
		roadmapskeleton.src

# ... object files ............................
SLOFILES=   $(SLO)$/unodialogsample.obj \
            $(SLO)$/roadmapskeleton.obj \
            $(SLO)$/roadmapskeletonpages.obj \
            $(SLO)$/udlg_module.obj \
            $(SLO)$/udlg_services.obj \

# --- library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1VERSIONMAP=$(TARGET).map

SHL1STDLIBS= \
        $(CPPULIB)          \
		$(CPPUHELPERLIB)    \
		$(COMPHELPERLIB)    \
		$(UNOTOOLSLIB)      \
		$(TOOLSLIB)         \
        $(SALLIB)           \
		$(SVTOOLLIB)        \
        $(VCLLIB)

SHL1LIBS=       $(SLB)$/$(TARGET).lib
SHL1IMPLIB=     i$(TARGET)
SHL1DEPN=       $(SHL1LIBS)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

DEF1NAME=       $(SHL1TARGET)

# --- .res files -------------------------------

RES1FILELIST=\
	$(SRS)$/$(TARGET).srs

RESLIB1NAME=$(TARGET)
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES=$(RES1FILELIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk

