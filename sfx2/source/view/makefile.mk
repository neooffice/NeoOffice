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
# Modified February 2007 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************



PRJ=..$/..

PRJNAME=			sfx2
TARGET=				view
ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = \
                view.src

SLOFILES =	\
                $(SLO)$/ipclient.obj \
		$(SLO)$/viewsh.obj \
		$(SLO)$/frmload.obj \
		$(SLO)$/frame.obj \
		$(SLO)$/printer.obj \
		$(SLO)$/prnmon.obj \
		$(SLO)$/viewprn.obj \
		$(SLO)$/viewfac.obj \
		$(SLO)$/orgmgr.obj \
		$(SLO)$/viewfrm.obj \
		$(SLO)$/impframe.obj \
		$(SLO)$/topfrm.obj \
		$(SLO)$/sfxbasecontroller.obj \
		$(SLO)$/userinputinterception.obj

.IF "$(GUIBASE)" == "java"
SLOFILES += $(SLO)$/topfrm_cocoa.obj
.ENDIF

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

