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
TARGET=unoxml
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

.IF "$(UPD)" == "310"
PREPENDLIBS=$(PRJ)$/..$/cppuhelper$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/salhelper$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/sax$/$(INPATH)$/lib

# Link to modified libraries
SOLARLIB:=-L$(PREPENDLIBS) $(SOLARLIB)
SOLARLIBDIR:=$(PREPENDLIBS) -L$(SOLARLIBDIR)
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

SLOFILES =	\
   $(SLO)$/services.obj


SHL1DEPN=   makefile.mk
SHL1OBJS=   $(SLOFILES) 

SHL1TARGET=	$(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1LIBS= \
    $(SLB)$/domimpl.lib \
    $(SLB)$/xpathimpl.lib \
    $(SLB)$/eventsimpl.lib

SHL1STDLIBS= \
    $(UCBHELPERLIB) \
    $(LIBXML2LIB) \
	$(TOOLSLIB)	\
	$(COMPHELPERLIB)	\
	$(CPPUHELPERLIB)	\
	$(CPPULIB)	\
	$(SAXLIB) \
	$(SALLIB)\
	$(EXPATASCII3RDLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

