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
# Modified March 2013 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=package
TARGET=xstor

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/ohierarchyholder.obj\
        $(SLO)$/ocompinstream.obj\
        $(SLO)$/oseekinstream.obj\
        $(SLO)$/owriteablestream.obj\
        $(SLO)$/xstorage.obj\
        $(SLO)$/xfactory.obj\
        $(SLO)$/disposelistener.obj\
        $(SLO)$/switchpersistencestream.obj\
        $(SLO)$/register.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
	$(SALLIB) 	\
	$(CPPULIB)	\
	$(CPPUHELPERLIB)	\
	$(COMPHELPERLIB)

.IF "$(GUIBASE)" == "java" && "$(OS)" == "MACOSX"
SHL1STDLIBS+=-lcrypto
.ENDIF		# "$(GUIBASE)" == "java" && "$(OS)" == "MACOSX"

SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(SHL1TARGET).dxp

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

