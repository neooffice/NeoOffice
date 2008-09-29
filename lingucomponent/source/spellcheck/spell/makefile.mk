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
# Modified July 2007 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ = ..$/..$/..

PRJNAME	= lingucomponent
TARGET	= spell
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

.IF "$(SYSTEM_HUNSPELL)" != "YES"
HUNSPELL_CFLAGS += -I$(SOLARINCDIR)$/hunspell
.ENDIF

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

.IF "$(PRODUCT_NAME)" != ""
CDEFS += -DPRODUCT_NAME='"$(PRODUCT_NAME)"'
.ENDIF

.IF "$(GUIBASE)" == "java"
OBJCFLAGS+=-fobjc-exceptions
.ENDIF

# --- Files --------------------------------------------------------

CXXFLAGS += -I$(PRJ)$/source$/lingutil $(HUNSPELL_CFLAGS)
CFLAGSCXX += -I$(PRJ)$/source$/lingutil $(HUNSPELL_CFLAGS)
CFLAGSCC += -I$(PRJ)$/source$/lingutil $(HUNSPELL_CFLAGS)

EXCEPTIONSFILES=	\
		$(SLO)$/sprophelp.obj\
		$(SLO)$/sspellimp.obj

SLOFILES=	\
		$(SLO)$/sprophelp.obj\
		$(SLO)$/sreg.obj\
		$(SLO)$/sspellimp.obj

.IF "$(GUIBASE)" == "java"
SLOFILES+=	\
		$(SLO)$/sspellimp_cocoa.obj
.ENDIF

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
		$(CPPULIB) 	 \
		$(CPPUHELPERLIB) 	 \
        $(I18NISOLANGLIB)   \
		$(VOSLIB)		\
		$(TOOLSLIB)		\
		$(SVTOOLLIB)	\
		$(SVLLIB)		\
		$(VCLLIB)		\
		$(SALLIB)		\
		$(UCBHELPERLIB)	\
		$(UNOTOOLSLIB)	\
		$(LNGLIB) \
		$(ULINGULIB) \
		$(ICUUCLIB) \
		$(HUNSPELLLIB)

.IF "$(GUIBASE)" == "java"
SHL1STDLIBS+= -framework AppKit
.ENDIF

# build DLL
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP= $(TARGET).map

# build DEF file
DEF1NAME	 =$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

