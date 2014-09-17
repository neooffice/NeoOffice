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
# Modified November 2008 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..
PRJNAME=shell
TARGET=cmdmail
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST=$(TARGET)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

DLLPRE=

SLOFILES= \
    $(SLO)$/cmdmailsuppl.obj \
    $(SLO)$/cmdmailmsg.obj \
    $(SLO)$/cmdmailentry.obj
.IF "$(GUIBASE)" == "java"
SLOFILES+= \
    $(SLO)$/cmdmailsuppl_cocoa.obj
.ENDIF		# "$(GUIBASE)" == "java"
SHL1OBJS=$(SLOFILES) 
			
SHL1TARGET=$(TARGET).uno
SHL1IMPLIB=i$(TARGET)

SHL1STDLIBS=$(CPPULIB)\
			$(CPPUHELPERLIB)\
			$(SALLIB)

.IF "$(GUIBASE)" == "java"
SHL1STDLIBS+=-framework AppKit
.ENDIF		# "$(GUIBASE)" == "java"

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

