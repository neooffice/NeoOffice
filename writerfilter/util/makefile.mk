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
PRJ=..
PRJNAME=writerfilter
.IF "$(GUI)" == "OS2"
TARGET=wfilt
.ELSE
TARGET=writerfilter
.ENDIF
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION

.IF "$(UPD)" == "310"
I18NPAPERLIB=-li18npaper$(DLLPOSTFIX)

PREPENDLIBS=$(PRJ)$/..$/comphelper$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/cppuhelper$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/goodies$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/i18npool$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/salhelper$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/sax$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/oox$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/svtools$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/svx$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/tools$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/unotools$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/vcl$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/xmloff$/$(INPATH)$/lib

# Link to modified libraries
SOLARLIB:=-L$(PREPENDLIBS) $(SOLARLIB)
SOLARLIBDIR:=$(PREPENDLIBS) -L$(SOLARLIBDIR)
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=  \
    $(SLB)$/ooxml.lib \
	$(SLB)$/resourcemodel.lib \
    $(SLB)$/dmapper.lib \
    $(SLB)$/filter.lib

.IF "$(UPD)" == "310"
LIB1FILES += \
    $(SLB)$/rtftok.lib
.ELSE	# "$(UPD)" == "310"
LIB1FILES += \
    $(SLB)$/doctok.lib
.ENDIF		# "$(UPD)" == "310"

SHL1LIBS=$(SLB)$/$(TARGET).lib


SHL1TARGET=$(TARGET)$(DLLPOSTFIX)
SHL1STDLIBS=\
    $(I18NISOLANGLIB) \
    $(I18NPAPERLIB) \
    $(SOTLIB) \
    $(TOOLSLIB) \
    $(UNOTOOLSLIB) \
    $(CPPUHELPERLIB)    \
    $(COMPHELPERLIB)    \
    $(CPPULIB)          \
    $(SALLIB)			\
    $(OOXLIB)

.IF "$(UPD)" == "310"
SHL1STDLIBS += \
	$(GOODIESLIB) \
	$(SAXLIB) \
	$(SFXLIB) \
	$(SVXCORELIB) \
	$(SVXMSFILTERLIB) \
	$(VCLLIB) \
	$(LIBXML2LIB)
.ENDIF		# "$(UPD)" == "310"


SHL1DEPN=
SHL1IMPLIB= i$(SHL1TARGET)
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=$(SOLARENV)/src/component.map

DEF1NAME=$(SHL1TARGET)


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(UPD)" != "310"
ALLTAR : $(MISC)/writerfilter.component

$(MISC)/writerfilter.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt writerfilter.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt writerfilter.component
ALLTAR : $(MISC)/writerfilter.component
.ENDIF		# "$(UPD)" != "310"
