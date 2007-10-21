#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile$
#
#   $Revision$
#
#   last change: $Author$ $Date$
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#     Modified October 2007 by Patrick Luby. NeoOffice is distributed under
#     GPL only under modification term 3 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=goodies
TARGET=ipict
DEPTARGET=vipict

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ----------------------------------------------------------

.IF "$(editdebug)"!="" || "$(EDITDEBUG)"!=""
CDEFS+= -DEDITDEBUG
.ENDIF

SLOFILES =  $(SLO)$/ipict.obj

# ==========================================================================

SHL1TARGET=     ipt$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB=     ipict
SHL1STDLIBS=    $(VCLLIB) $(TOOLSLIB) $(SALLIB)
SHL1LIBS=       $(SLB)$/ipict.lib

.IF "$(GUI)" != "UNX"
SHL1OBJS=       $(SLO)$/ipict.obj
.ENDIF

.IF "$(GUIBASE)" == "java"
SHL1STDLIBS+=   -framework ApplicationServices
.ENDIF

SHL1VERSIONMAP=exports.map
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# ==========================================================================

.INCLUDE :  target.mk

