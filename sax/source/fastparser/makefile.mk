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
#  Modified September 2014 by Patrick Luby. NeoOffice is distributed under
#  GPL only under Section 4 of the Apache License v2.0.
#  
#**************************************************************


PRJ=..$/..

PRJNAME=sax
TARGET=fastsax.uno
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE = 

.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

.IF "$(UPD)" == "310"
INCLOCAL+= \
	-I$(PRJ)$/..$/expat$/$(INPATH)$/misc$/build$/expat-2.1.0$/lib \
	-I$(PRJ)$/..$/offapi$/$(INPATH)$/inc$/cssutil \
	-I$(PRJ)$/..$/offapi$/$(INPATH)$/inc$/cssxmlsax \
	-I$(PRJ)$/..$/sal$/inc \
	-I$(PRJ)$/..$/salhelper$/inc

PREPENDLIBS=$(PRJ)$/..$/salhelper$/$(INPATH)$/lib

# Link to modified libraries
SOLARLIB:=-L$(PREPENDLIBS) $(SOLARLIB)
SOLARLIBDIR:=$(PREPENDLIBS) -L$(SOLARLIBDIR)
.ENDIF		# "$(UPD)" == "310"

#-----------------------------------------------------------

SLOFILES =\
		$(SLO)$/facreg.obj

.IF "$(UPD)" != "310"
SLOFILES += \
		$(SLO)$/fastparser.obj \
		$(SLO)$/xml2utf.obj
.ENDIF		# "$(UPD)" != "310"

.IF "$(GUI)" == "OS2"
SHL1TARGET= fastsax
.ELSE
SHL1TARGET= $(TARGET)
.ENDIF
SHL1IMPLIB= i$(TARGET)

SHL1STDLIBS= \
		$(SALLIB)  \
		$(SAXLIB)  \
		$(CPPULIB) \
		$(CPPUHELPERLIB)

.IF "$(UPD)" != "310"
SHL1STDLIBS += \
		$(EXPATASCII3RDLIB)
.ENDIF		# "$(UPD)" != "310"

SHL1DEPN=
SHL1VERSIONMAP=	$(SOLARENV)$/src$/component.map
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(UPD)" != "310"
ALLTAR : $(MISC)/fastsax.component

$(MISC)/fastsax.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        fastsax.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt fastsax.component
.ENDIF		# "$(UPD)" == "310"
