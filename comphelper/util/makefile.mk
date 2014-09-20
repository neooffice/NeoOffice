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
# Modified September 2014 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..
PRJNAME=comphelper
TARGET=comphelper

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/version.mk

# --- Library -----------------------------------

LIB1TARGET=	$(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/container.lib		\
			$(SLB)$/evtattmgr.lib		\
			$(SLB)$/misc.lib			\
			$(SLB)$/processfactory.lib	\
			$(SLB)$/property.lib		\
			$(SLB)$/streaming.lib		\
            $(SLB)$/compare.lib         \
            $(SLB)$/officeinstdir.lib	\
            $(SLB)$/xml.lib

SHL1TARGET=$(COMPHLP_TARGET)$(COMPHLP_MAJOR)$(COMID)
.IF "$(GUI)" == "OS2"
SHL1TARGET=comph$(COMPHLP_MAJOR)
.ENDIF
SHL1STDLIBS= \
	$(SALLIB) \
	$(SALHELPERLIB) \
	$(CPPUHELPERLIB) \
	$(CPPULIB) \
	$(UCBHELPERLIB) \
	$(VOSLIB)

.IF "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"
SHL1STDLIBS+= \
	$(SVLLIB)
.ENDIF		# "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"

SHL1DEPN=
SHL1IMPLIB=	i$(COMPHLP_TARGET)
SHL1USE_EXPORTS=name
SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk
