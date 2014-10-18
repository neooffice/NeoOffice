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
TARGET=sax.uno
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ELSE
CFLAGS += -DXML_UNICODE
.ENDIF

.IF "$(UPD)" == "310"
ENVINCPRE += -I$(PRJ)$/..$/include
INCLOCAL += -I$(PRJ)$/..$/sal/inc -I$(INCCOM)$/cssutil -I$(INCCOM)$/cssxmlsax
# Link to modified libexpat*.a
SOLARLIB:=-L$(PRJ)$/..$/expat$/$(INPATH)$/lib $(SOLARLIB)
.ENDIF		# "$(UPD)" == "310"

#-----------------------------------------------------------

SLOFILES =\
		$(SLO)$/xml2utf.obj\
		$(SLO)$/attrlistimpl.obj\
		$(SLO)$/sax_expat.obj \
		$(SLO)$/saxwriter.obj

.IF "$(GUI)" == "OS2"
SHL1TARGET= sax_uno
.ELSE
SHL1TARGET= $(TARGET)
.ENDIF
SHL1IMPLIB= i$(TARGET)

SHL1STDLIBS= \
		$(SALLIB)  \
		$(CPPULIB) \
		$(CPPUHELPERLIB)\
		$(EXPAT3RDLIB)

SHL1DEPN=
SHL1VERSIONMAP=	$(SOLARENV)$/src$/component.map
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(UPD)" != "310"
ALLTAR : $(MISC)/sax.component $(MISC)/sax.inbuild.component

$(MISC)/sax.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        sax.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt sax.component

$(MISC)/sax.inbuild.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt sax.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_INBUILD_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt sax.component
.ENDIF		# "$(UPD)" != "310"

