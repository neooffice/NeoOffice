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
# Modified August 2008 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=dtrans
TARGET=dtransX11
TARGETTYPE=GUI

ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

.IF "$(GUIBASE)"=="aqua" || "$(GUIBASE)"=="java"

dummy:
	@echo "Nothing to build for Mac OS X"
 
.ELSE		# "$(GUIBASE)"=="aqua" || "$(GUIBASE)"=="java"

.IF "$(COM)$(CPU)" == "C50I" || "$(COM)$(CPU)" == "C52I"
NOOPTFILES=\
	$(SLO)$/X11_selection.obj
.ENDIF

SLOFILES=\
	$(SLO)$/X11_dndcontext.obj		\
	$(SLO)$/X11_transferable.obj	\
	$(SLO)$/X11_clipboard.obj		\
	$(SLO)$/X11_selection.obj		\
	$(SLO)$/X11_droptarget.obj		\
	$(SLO)$/X11_service.obj			\
	$(SLO)$/bmp.obj					\
	$(SLO)$/config.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS= $(LIBSTLPORT) $(CPPUHELPERLIB)
.ELSE
SHL1STDLIBS= $(CPPUHELPERLIB)
.ENDIF

SHL1STDLIBS+= \
		$(UNOTOOLSLIB)	\
		$(CPPULIB) 	\
		$(SALLIB)	\
		-lX11

SHL1DEPN=
SHL1IMPLIB=		i$(SHL1TARGET) 
SHL1OBJS=		$(SLOFILES)

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

.ENDIF		# "$(GUIBASE)"=="aqua" || "$(GUIBASE)"=="java"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
