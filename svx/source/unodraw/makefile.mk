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
# Modified May 2015 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=svx
TARGET=unodraw
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET)-core.lib
LIB1OBJFILES= \
		$(SLO)$/UnoGraphicExporter.obj \
		$(SLO)$/XPropertyTable.obj \
		$(SLO)$/UnoNameItemTable.obj \
		$(SLO)$/unoshape.obj	\
		$(SLO)$/unoipset.obj	\
		$(SLO)$/unoshap2.obj	\
		$(SLO)$/unoshap3.obj	\
		$(SLO)$/unoshap4.obj	\
		$(SLO)$/unopage.obj		\
		$(SLO)$/unoshtxt.obj	\
		$(SLO)$/unoprov.obj		\
		$(SLO)$/unomod.obj      \
		$(SLO)$/unonrule.obj	\
		$(SLO)$/unofdesc.obj	\
		$(SLO)$/unomlstr.obj	\
		$(SLO)$/unogtabl.obj	\
		$(SLO)$/unohtabl.obj	\
		$(SLO)$/unobtabl.obj	\
		$(SLO)$/unottabl.obj	\
		$(SLO)$/unomtabl.obj	\
		$(SLO)$/unodtabl.obj	\
		$(SLO)$/gluepts.obj     \
		$(SLO)$/tableshape.obj

.IF "$(UPD)" == "310"
LIB1OBJFILES += \
		$(SLO)$/unobrushitemhelper.obj
.ENDIF		# "$(UPD)" == "310"

LIB2TARGET= $(SLB)$/$(TARGET).lib
LIB2OBJFILES= \
		$(SLO)$/UnoNamespaceMap.obj \
		$(SLO)$/unopool.obj \
		$(SLO)$/unoctabl.obj	\
		$(SLO)$/unoshcol.obj	\
		$(SLO)$/recoveryui.obj

SLOFILES = $(LIB1OBJFILES) $(LIB2OBJFILES)

SRS1NAME=unodraw
SRC1FILES =  \
		unodraw.src

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

