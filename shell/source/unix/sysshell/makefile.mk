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
#     Modified February 2007 by Patrick Luby. NeoOffice is distributed under
#     GPL only under modification term 3 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=SHELL
TARGET=sysshell
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

.IF "$(SYSTEM_EXPAT)" == "YES"
CFLAGS+=-DSYSTEM_EXPAT
.ENDIF

# --- Files --------------------------------------------------------

LIB1OBJFILES=$(SLO)$/systemshell.obj
LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1ARCHIV=$(SLB)$/lib$(TARGET).a

SLOFILES=$(SLO)$/recently_used_file.obj \
         $(SLO)$/recently_used_file_handler.obj

.IF "$(GUIBASE)" == "java"
SLOFILES+=$(SLO)$/recently_used_file_handler_cocoa.obj
.ENDIF	# "$(GUIBASE)" == "java"

SHL1TARGET=recentfile

# static libs must come at end of linker list on MacOSX
.IF "$(OS)" == "MACOSX"
SHL1STDLIBS=$(SALLIB)\
	$(EXPATASCII3RDLIB)\
	$(CPPULIB)\
	$(CPPUHELPERLIB)\
	$(COMPHELPERLIB)
.ELSE
SHL1STDLIBS=$(EXPATASCII3RDLIB)\
	$(SALLIB)\
	$(CPPULIB)\
	$(CPPUHELPERLIB)\
	$(COMPHELPERLIB)
.ENDIF # MACOSX

.IF "$(GUIBASE)" == "java"
SHL1STDLIBS+=-framework CoreFoundation -framework Cocoa
.ENDIF	# "$(GUIBASE)" == "java"

SHL1LIBS=$(SLB)$/xmlparser.lib
SHL1OBJS=$(SLOFILES)
SHL1VERSIONMAP=recfile.map

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
