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
#   Patrick Luby, June 2003
# 
#   GNU General Public License Version 2.1
#   =============================================
#   Copyright 2003 by Patrick Luby (patrick.luby@planamesa.com)
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

PRJNAME=dtrans
TARGET=dtransjava
TARGETTYPE=GUI

ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)
LIBTARGET=NO

ENVCDEFS += -Iinc

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  ..$/cppumaker.mk

# ------------------------------------------------------------------

.IF "$(GUIBASE)"!="java"

dummy:
	@echo "Nothing to build for GUIBASE $(GUIBASE)"
 
.ELSE		# "$(GUIBASE)"!="java"

SLOFILES=\
	$(SLO)$/Class.obj \
	$(SLO)$/DTransClipboard.obj \
	$(SLO)$/DTransTransferable.obj \
	$(SLO)$/Object.obj \
	$(SLO)$/String.obj \
	$(SLO)$/Throwable.obj \
	$(SLO)$/java_clipboard.obj \
	$(SLO)$/java_dnd.obj \
	$(SLO)$/java_dndcontext.obj \
	$(SLO)$/java_service.obj \
	$(SLO)$/tools.obj

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)

SHL1STDLIBS= \
		$(SALLIB)	\
		$(TOOLSLIB)	\
		$(VOSLIB)	\
		$(CPPULIB) 	\
		$(CPPUHELPERLIB)	\
		$(COMPHELPERLIB)

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS += -framework Carbon -framework QuickTime
.ENDIF

SHL1DEPN=
SHL1IMPLIB=		i$(SHL1TARGET) 
SHL1OBJS=		$(SLOFILES)

APP1NOSAL=TRUE
APP1TARGET=testjavacb
APP1OBJS=$(SLO)$/test_javacb.obj
APP1STDLIBS=\
		$(SALLIB)	\
		$(TOOLSLIB)	\
		$(CPPULIB)			\
		$(CPPUHELPERLIB)	\
		$(COMPHELPERLIB)

.IF "$(OS)"=="MACOSX"
APP1STDLIBS += -framework QuickTime
.ENDIF

JARCLASSDIRS = com
JARTARGET = $(TARGET).jar
JARCOMPRESS = TRUE

.ENDIF		# "$(GUIBASE)"!="java"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
