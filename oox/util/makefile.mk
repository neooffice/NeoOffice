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



PRJ=..

PRJNAME=oox
TARGET=oox
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""

.IF "$(UPD)" == "310"
PREPENDLIBS=$(PRJ)$/..$/comphelper$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/salhelper$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/sax$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/svtools$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/svx$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/tools$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/vcl$/$(INPATH)$/lib \
	-L$(PRJ)$/..$/xmloff$/$(INPATH)$/lib

# Link to modified libcomphelp, libsax, libtl, and libxo
SOLARLIB:=-L$(PREPENDLIBS) $(SOLARLIB)
SOLARLIBDIR:=$(PREPENDLIBS) -L$(SOLARLIBDIR)
.ENDIF		# "$(UPD)" == "310"

# --- Allgemein ----------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	\
	$(SLB)$/token.lib\
    $(SLB)$/helper.lib\
    $(SLB)$/core.lib\
    $(SLB)$/ole.lib\
    $(SLB)$/ppt.lib\
    $(SLB)$/xls.lib\
    $(SLB)$/vml.lib\
    $(SLB)$/drawingml.lib\
    $(SLB)$/diagram.lib\
    $(SLB)$/chart.lib\
    $(SLB)$/table.lib\
    $(SLB)$/shape.lib\
    $(SLB)$/dump.lib\
    $(SLB)$/docprop.lib

.IF "$(UPD)" == "310"
LIB1FILES+= \
    $(SLB)$/crypto.lib \
    $(SLB)$/export.lib \
    $(SLB)$/mathml.lib
.ENDIF		# "$(UPD)" == "310"

# --- Shared-Library -----------------------------------------------

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)
SHL1USE_EXPORTS=name

SHL1STDLIBS= \
		$(CPPULIB)		\
		$(CPPUHELPERLIB)\
		$(COMPHELPERLIB)\
		$(RTLLIB)		\
		$(SALLIB)		\
		$(BASEGFXLIB)	\
		$(SAXLIB)       \
        $(XMLSCRIPTLIB)

# link openssl, copied this bit from ucb/source/ucp/webdav/makefile.mk
.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(OPENSSLLIB)
.ELSE # WNT
.IF "$(OS)"=="SOLARIS"
SHL1STDLIBS+= -lnsl -lsocket -ldl
.ENDIF # SOLARIS
.IF "$(SYSTEM_OPENSSL)"=="YES"
SHL1STDLIBS+= $(OPENSSLLIB)
.ELSE
SHL1STDLIBS+= $(OPENSSLLIBST)
.ENDIF
.ENDIF # WNT

.IF "$(UPD)" == "310"
SHL1STDLIBS += \
		$(GOODIESLIB) \
		$(SOTLIB) \
		$(SVTOOLLIB) \
		$(SVXCORELIB) \
		$(SVXMSFILTERLIB) \
		$(TOOLSLIB) \
		$(UNOTOOLSLIB) \
		$(VCLLIB) \
		$(XMLOFFLIB)
.ENDIF		# "$(UPD)" == "310"

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=   $(LIB1TARGET)
DEF1NAME    =$(SHL1TARGET)
DEFLIB1NAME =$(TARGET)

# --- Targets ----------------------------------------------------------
.ENDIF # L10N_framework

.INCLUDE :  target.mk

.IF "$(UPD)" != "310"
ALLTAR : $(MISC)/oox.component

$(MISC)/oox.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        oox.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt oox.component
.ENDIF		# "$(UPD)" != "310"
