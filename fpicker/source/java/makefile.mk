#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile$
#
#   $Revision$
#
#   last change: $Author$ $Date$
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..

PRJNAME=fpicker
TARGET=fps_java.uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

DLLPRE=

# ------------------------------------------------------------------

.IF "$(GUIBASE)" != "java"

dummy:
	@echo "Nothing to build. GUIBASE == $(GUIBASE)"

.ELSE	# "$(GUIBASE)" != "java"

# --- Files --------------------------------------------------------

SLOFILES =\
		$(SLO)$/cocoa_dialog.obj				\
		$(SLO)$/java_filepicker.obj				\
		$(SLO)$/java_folderpicker.obj			\
		$(SLO)$/java_service.obj

SHL1TARGET=	$(TARGET)
SHL1OBJS=	$(SLOFILES)
SHL1STDLIBS=\
	$(VCLLIB) \
	$(TOOLSLIB) \
	$(CPPUHELPERLIB) \
	$(CPPULIB) \
	$(SALLIB) \
	-framework Cocoa

.ENDIF	# "$(GUIBASE)" != "java"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
