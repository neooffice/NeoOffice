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
#  Modified October 2014 by Patrick Luby. NeoOffice is distributed under
#  GPL only under Section 4 of the Apache License v2.0.
#  
#**************************************************************



PRJ=..$/..

PRJNAME=sax
TARGET=sax
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(UPD)" == "310"
ENVINCPRE += -I$(PRJ)$/..$/include
INCLOCAL += -I$(PRJ)$/..$/sal/inc -I$(INCCOM)$/cssutil -I$(INCCOM)$/cssxmlsax
# Link to modified libexpat*.a and libuno_salhelper
SOLARLIB:=-L$(PRJ)$/..$/expat$/$(INPATH)$/lib -L$(PRJ)$/..$/salhelper$/$(INPATH)$/lib $(SOLARLIB)
SOLARLIBDIR:=$(PRJ)$/..$/expat$/$(INPATH)$/lib -L$(PRJ)$/..$/salhelper$/$(INPATH)$/lib -L$(SOLARLIBDIR)
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

SLOFILES =	\
		$(SLO)$/converter.obj				\
		$(SLO)$/fastattribs.obj				\
		$(SLO)$/fastserializer.obj			\
		$(SLO)$/fshelper.obj

.IF "$(UPD)" == "310"
SLOFILES += \
		$(SLO)$/fastparser.obj \
		$(SLO)$/xml2utf.obj
.ENDIF		# "$(UPD)" == "310"

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)

SHL1STDLIBS= \
				$(CPPULIB)		\
				$(CPPUHELPERLIB)\
				$(COMPHELPERLIB)\
				$(RTLLIB)		\
				$(SALLIB)		\
				$(ONELIB)

.IF "$(UPD)" == "310"
SHL1STDLIBS+= \
		$(EXPAT3RDLIB) \
		$(SALHELPERLIB)
.ENDIF		# "$(UPD)" == "310"

SHL1DEPN=
SHL1OBJS=       $(SLOFILES)
SHL1USE_EXPORTS=name
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
DEF1NAME=		$(SHL1TARGET)
DEFLIB1NAME=    $(TARGET)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(UPD)" == "310"
ALLTAR: $(SLO)$/fastparser.obj

$(SLO)$/fastparser.obj:
	cd ../fastparser && dmake $(MFLAGS) $(MAKEFILE) $@
.ENDIF		# "$(UPD)" == "310"
