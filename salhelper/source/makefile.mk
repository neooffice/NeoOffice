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
# Modified October 2014 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************
PRJ=..

PRJNAME=salhelper
TARGET=salhelper

ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE
USE_DEFFILE=TRUE

.IF "$(OS)" != "WNT" && "$(GUI)"!="OS2"
UNIXVERSIONNAMES=UDK
.ENDIF # WNT

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(UPD)" == "310"
ENVINCPRE += -I$(PRJ)$/..$/include
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

SLOFILES=	\
	$(SLO)$/condition.obj \
	$(SLO)$/dynload.obj \
	$(SLO)$/simplereferenceobject.obj

# SCO: the linker does know about weak symbols, but we can't ignore multiple defined symbols
.IF "$(OS)"=="SCO"
SLOFILES+=$(SLO)$/staticmb.obj
.ENDIF

.IF "$(UPD)" == "310"
SLOFILES+=$(SLO)$/thread.obj
.ENDIF		# "$(UPD)" == "310"

.IF "$(GUI)" == "WNT"
SHL1TARGET=	$(TARGET)$(UDK_MAJOR)$(COMID)
.ELIF "$(GUI)" == "OS2"
SHL1TARGET=	salhelp$(UDK_MAJOR)
.ELSE
SHL1TARGET=	uno_$(TARGET)$(COMID)
.ENDIF

SHL1STDLIBS=$(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=  URELIB

DEF1NAME=	$(SHL1TARGET)

.IF "$(COMNAME)"=="msci"
SHL1VERSIONMAP=msci.map
.ELIF "$(GUI)"=="OS2"
SHL1VERSIONMAP=gcc3os2.map
.ELIF "$(COMNAME)"=="sunpro5"
SHL1VERSIONMAP=sols.map
.ELIF "$(COMNAME)"=="gcc3"
SHL1VERSIONMAP=gcc3.map
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

