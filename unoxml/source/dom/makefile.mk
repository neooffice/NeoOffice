#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.
#  
#  $RCSfile$
#  $Revision$
#  
#  This file is part of NeoOffice.
#  
#  NeoOffice is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 3
#  only, as published by the Free Software Foundation.
#  
#  NeoOffice is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License version 3 for more details
#  (a copy is included in the LICENSE file that accompanied this code).
#  
#  You should have received a copy of the GNU General Public License
#  version 3 along with NeoOffice.  If not, see
#  <http://www.gnu.org/licenses/gpl-3.0.txt>
#  for a copy of the GPLv3 License.
#  
#  Modified November 2014 by Patrick Luby. NeoOffice is distributed under
#  GPL only under Section 4 of the Apache License v2.0.
#  
#**************************************************************

PRJ=..$/..

PRJNAME=unoxml
TARGET=domimpl

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

.IF "$(UPD)" == "310"
INCLOCAL += \
	-I$(PRJ)$/..$/sal/inc \
	-I$(PRJ)$/..$/sax/inc
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

SLOFILES =	\
    $(SLO)$/attr.obj \
    $(SLO)$/cdatasection.obj \
    $(SLO)$/characterdata.obj \
    $(SLO)$/comment.obj \
    $(SLO)$/document.obj \
    $(SLO)$/documentbuilder.obj \
    $(SLO)$/documentfragment.obj \
    $(SLO)$/documenttype.obj \
    $(SLO)$/element.obj \
    $(SLO)$/entity.obj \
    $(SLO)$/entityreference.obj \
    $(SLO)$/node.obj \
    $(SLO)$/notation.obj \
    $(SLO)$/processinginstruction.obj \
    $(SLO)$/text.obj \
    $(SLO)$/domimplementation.obj \
    $(SLO)$/elementlist.obj \
    $(SLO)$/childlist.obj \
    $(SLO)$/notationsmap.obj \
    $(SLO)$/entitiesmap.obj \
    $(SLO)$/attributesmap.obj \
    $(SLO)$/saxbuilder.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


