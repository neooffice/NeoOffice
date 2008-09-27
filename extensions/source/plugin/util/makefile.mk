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
# version 3 along with OpenOffice.org.  If not, see
# <http://www.gnu.org/licenses/gpl-3.0.txt>
# for a copy of the GPLv3 License.
#
# Modified December 2007 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************
PRJ=..$/..$/..

PRJNAME=extensions
TARGET=pl
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(WITH_MOZILLA)" != "NO"

LIB1TARGET = $(SLB)$/plall.lib
LIB1FILES  = \
	$(SLB)$/plbase.lib	\
	$(SHL1LINKLIB)

.IF "$(GUI)" == "UNX"
.IF "$(GUIBASE)"=="java"
SHL1LINKLIB = $(SLB)$/pljava.lib
.ELSE	# "$(GUIBASE)"=="java"
.IF "$(GUIBASE)"=="aqua"
.IF "$(WITH_MOZILLA)"=="YES"
SHL1LINKLIB = $(SLB)$/plaqua.lib
.ENDIF
.ELSE
SHL1LINKLIB = $(SLB)$/plunx.lib
.ENDIF # $(GUIBASE)==aqua
.ENDIF		# "$(GUIBASE)"=="java"
.IF "$(OS)" == "SOLARIS"
SHL1OWNLIBS = -lsocket
.ENDIF # SOLARIS
.ENDIF # UNX

.IF "$(GUI)" == "WNT"
SHL1LINKLIB = $(SLB)$/plwin.lib
SHL1OWNLIBS = \
	$(VERSIONLIB)	\
	$(OLE32LIB)	\
	$(ADVAPI32LIB)
.ENDIF # WNT

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
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

.IF "$(OS)"=="MACOSX" && "$(GUIBASE)"=="unx"
SHL1STDLIBS+= -lX11
.ENDIF

SHL1STDLIBS+=$(SHL1OWNLIBS)

.ENDIF # $(WITH_MOZILLA) != "NO"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk



