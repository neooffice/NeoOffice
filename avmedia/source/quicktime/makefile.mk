##########################################################################
# 
#   $RCSfile$
# 
#   $Revision$
# 
#   last change: $Author$ $Date$
# 
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
# 
#          - GNU General Public License Version 2.1
# 
#   Patrick Luby, May 2006
# 
#   GNU General Public License Version 2.1
#   =============================================
#   Copyright 2006 Planamesa Inc.
# 
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public
#   License version 2.1, as published by the Free Software Foundation.
# 
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
# 
#   You should have received a copy of the GNU General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
# 
##########################################################################

PRJ=..$/..
PRJNAME=avmediaquicktime
TARGET=avmediaquicktime

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Files ----------------------------------

.IF "$(OS)" == "MACOSX"

SLOFILES= \
	$(SLO)$/quicktimemanager.obj \
	$(SLO)$/quicktimeplayer.obj \
	$(SLO)$/quicktimeuno.obj

EXCEPTIONSFILES= \
	$(SLO)$/quicktimeuno.obj

SHL1TARGET=$(TARGET)
SHL1STDLIBS= \
	$(CPPULIB) \
	$(SALLIB) \
	$(COMPHELPERLIB) \
	$(CPPUHELPERLIB) \
	$(UNOTOOLSLIB) \
	$(TOOLSLIB) \
	$(VOSLIB) \
	$(VCLLIB)

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

SHL1STDLIBS += -framework CoreFoundation -framework Carbon

.ENDIF	# "$(OS)" == "MACOSX"

# --- Targets -----------------------------------
.INCLUDE :  	target.mk
