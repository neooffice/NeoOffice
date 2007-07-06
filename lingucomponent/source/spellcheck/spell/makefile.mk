#*************************************************************************
#
#   $RCSfile$
#
#   $Revision$
#
#   last change: $Author$ $Date$
#
#   The Contents of this file are made available subject to
#   the terms of GNU General Public License Version 2.1.
#
#
#     GNU General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     General Public License for more details.
#
#     You should have received a copy of the GNU General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#     Modified July 2007 by Patrick Luby. NeoOffice is distributed under
#     GPL only under modification term 3 of the LGPL.
#
#*************************************************************************

PRJ = ..$/..$/..

PRJNAME	= lingucomponent
TARGET	= spell
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

.IF "$(MYSPELLLIB)"==""
.ELSE
@echo "Build Hunspell instead of system Myspell."
.ENDIF

.IF "$(HUNSPELLLIB)"==""
.IF "$(GUI)"=="UNX"
HUNSPELLLIB=-lhunspell
.ENDIF # unx
.IF "$(GUI)"=="WNT"
HUNSPELLLIB=hunspell.lib
.ENDIF # wnt
.ENDIF

.IF "$(ULINGULIB)"==""
.IF "$(GUI)"=="UNX"
ULINGULIB=-lulingu
.ENDIF # unx
.IF "$(GUI)"=="WNT"
ULINGULIB=libulingu.lib
.ENDIF # wnt
.ENDIF




#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_HUNSPELL)" != "YES"
CXXFLAGS += -I..$/hunspell -I..$/..$/lingutil
CFLAGSCXX += -I..$/hunspell -I..$/..$/lingutil
CFLAGSCC += -I..$/hunspell  -I..$/..$/lingutil
.ELSE
CXXGLAGS += $(HUNSPELL_CFLAGS)
CFLAGSCXX += $(HUNSPELL_CFLAGS)
CFLAGSCC += $(HUNSPELL_CFLAGS)
.ENDIF

.IF "$(header)" == ""

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

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)

SHL1STDLIBS= \
		$(CPPULIB) 	 \
		$(CPPUHELPERLIB) 	 \
		$(UNOLIB)		\
		$(VOSLIB)		\
		$(TOOLSLIB)		\
		$(SVTOOLLIB)	\
		$(SVLLIB)		\
		$(VCLLIB)		\
		$(SALLIB)		\
		$(UCBHELPERLIB)	\
		$(UNOTOOLSLIB)	\
		$(LNGLIB) \
                $(HUNSPELLLIB)

.IF "$(SYSTEM_HUNSPELL)" != "YES"
SHL1STDLIBS+=   $(ULINGULIB)
.ENDIF

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
#DEF1DEPN    =$(MISC)$/$(SHL1TARGET).flt
#DEFLIB1NAME =$(TARGET)
#DEF1DES     =Linguistic2 main DLL
DEF1EXPORTFILE=	exports.dxp

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

