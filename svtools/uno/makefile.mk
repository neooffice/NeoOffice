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

PRJ=..
PRJNAME=svtools
TARGET=svtmisc.uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
DLLPRE=

.IF "$(UPD)" == "310"
PREPENDLIBS=$(PRJ)$/..$/comphelper$/$(INPATH)$/lib

# Link to modified libraries
SOLARLIB:=-L$(PREPENDLIBS) $(SOLARLIB)
SOLARLIBDIR:=$(PREPENDLIBS) -L$(SOLARLIBDIR)
.ENDIF		# "$(UPD)" == "310"

# --- Files -------------------------------------

SLOFILES=	\
	$(SLO)$/addrtempuno.obj \
	$(SLO)$/miscservices.obj \
	$(SLO)$/pathservice.obj

SHL1TARGET=	$(TARGET)
SHL1IMPLIB=	i$(TARGET)

SHL1OBJS= \
	$(SLO)$/svtdata.obj \
	$(SLOFILES)

SHL1LIBS=	\
	$(SLB)$/filter.uno.lib

SHL1STDLIBS=\
	$(SVTOOLLIB) \
	$(TKLIB) \
	$(VCLLIB) \
	$(SVLLIB) \
	$(UNOTOOLSLIB) \
	$(TOOLSLIB) \
	$(COMPHELPERLIB) \
	$(VOSLIB) \
	$(CPPUHELPERLIB) \
	$(CPPULIB) \
	$(SALLIB)

SHL1VERSIONMAP=exports.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

