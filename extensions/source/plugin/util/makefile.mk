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
#     Modified December 2005 by Patrick Luby. NeoOffice is distributed under
#     GPL only under modification term 3 of the LGPL.
#
#*************************************************************************
PRJ=..$/..$/..

PRJNAME=extensions
TARGET=pl
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------
LIB1TARGET = $(SLB)$/plall.lib
LIB1FILES  = \
	$(SLB)$/plbase.lib	\
	$(SHL1LINKLIB)

.IF "$(GUI)" == "UNX"
SHL1LINKLIB = $(SLB)$/plunx.lib
.IF "$(OS)" == "SOLARIS"
SHL1OWNLIBS = -lsocket
.ENDIF # SOLARIS
.ENDIF # UNX

.IF "$(GUI)" == "WNT"
SHL1LINKLIB = $(SLB)$/plwin.lib
SHL1OWNLIBS = \
	version.lib	\
	ole32.lib	\
	advapi32.lib
.ENDIF # WNT

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1LIBS=$(LIB1TARGET)

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS= \
	$(LIBSTLPORT)		\
	$(TKLIB)
.ELSE
SHL1STDLIBS= \
	$(TKLIB)
.ENDIF

SHL1STDLIBS+= \
	$(VCLLIB)			\
	$(SVLLIB)			\
	$(TOOLSLIB)			\
	$(VOSLIB)			\
	$(UCBHELPERLIB)		\
	$(CPPUHELPERLIB)	\
	$(CPPULIB)			\
	$(SALLIB)

.IF "$(OS)"=="MACOSX"
.IF "$(GUIBASE)"!="java"
SHL1STDLIBS+= -lX11
.ENDIF
.ENDIF

SHL1STDLIBS+=$(SHL1OWNLIBS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



