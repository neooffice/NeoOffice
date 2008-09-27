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
# Modified September 2008 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************
PRJ=..$/..$/..

PRJNAME= jvmfwk

TARGET = plugin

ENABLE_EXCEPTIONS=TRUE

LIBTARGET=NO

UNOCOMPONENT1=sunjavaplugin

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

# turn off optimizations on MACOSX due to optimization errors in gcc 4.0.1
.IF "$(OS)" == "MACOSX"
CDEFS+=-O0
.ENDIF

#.INCLUDE :  ..$/cppumaker.mk
.IF "$(SOLAR_JAVA)"!=""

SLOFILES= \
	$(SLO)$/sunversion.obj \
	$(SLO)$/sunjavaplugin.obj \
	$(SLO)$/vendorbase.obj \
	$(SLO)$/util.obj \
	$(SLO)$/sunjre.obj \
	$(SLO)$/gnujre.obj \
	$(SLO)$/vendorlist.obj \
	$(SLO)$/otherjre.obj 

LIB1OBJFILES= $(SLOFILES)



LIB1TARGET=$(SLB)$/$(UNOCOMPONENT1).lib

SHL1TARGET=	$(UNOCOMPONENT1)  


SHL1STDLIBS= \
		$(CPPULIB) \
		$(CPPUHELPER) \
		$(SALLIB) \
		$(SALHELPERLIB)
		

.IF "$(GUI)" == "WNT"
.IF "$(COM)"!="GCC"
SHL1STDLIBS += uwinapi.lib advapi32.lib
.ELSE
SHL1STDLIBS += -luwinapi -ladvapi32 
.ENDIF # GCC
.ENDIF #WNT

.IF "$(GUIBASE)" == "java"
SHL1STDLIBS += -framework Carbon
.ENDIF # "$(GUIBASE)" == "java"

SHL1VERSIONMAP = sunjavaplugin.map
SHL1DEPN=
SHL1IMPLIB=	i$(UNOCOMPONENT1)
SHL1LIBS=	$(LIB1TARGET) 
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)
SHL1RPATH=  URELIB

JAVACLASSFILES= \
    $(CLASSDIR)$/JREProperties.class					

JAVAFILES = $(subst,$(CLASSDIR)$/, $(subst,.class,.java $(JAVACLASSFILES))) 

.ENDIF # SOLAR_JAVA



# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(GUI)"=="WNT"
BOOTSTRAPFILE=$(BIN)$/sunjavaplugin.ini
.ELSE
BOOTSTRAPFILE=$(BIN)$/sunjavapluginrc
.ENDIF


$(BOOTSTRAPFILE): sunjavapluginrc
	-$(COPY) $< $@


ALLTAR: \
	$(BOOTSTRAPFILE)

