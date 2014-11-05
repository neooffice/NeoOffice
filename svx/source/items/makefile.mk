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
# Modified November 2014 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PROJECTPCH4DLL=TRUE
PROJECTPCH=svxpch
PROJECTPCHSOURCE=$(PRJ)$/util$/svxpch
#ENABLE_EXCEPTIONS=TRUE

PRJNAME=svx
TARGET=items
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(UPD)" == "310"
INCLOCAL+= -I$(PRJ)$/..$/offapi/$(INPATH)$/inc$/csstable
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

SRS1NAME=svxitems
SRC1FILES =  \
		svxerr.src		\
		svxitems.src

LIB1TARGET= $(SLB)$/$(TARGET)-core.lib
LIB1OBJFILES= \
		$(SLO)$/writingmodeitem.obj \
		$(SLO)$/frmitems.obj \
		$(SLO)$/paraitem.obj \
		$(SLO)$/textitem.obj \
		$(SLO)$/flditem.obj \
		$(SLO)$/svxfont.obj \
		$(SLO)$/drawitem.obj	\
		$(SLO)$/itemtype.obj	\
		$(SLO)$/chrtitem.obj	\
		$(SLO)$/bulitem.obj \
		$(SLO)$/e3ditem.obj \
		$(SLO)$/numitem.obj \
		$(SLO)$/grfitem.obj \
		$(SLO)$/clipfmtitem.obj \
		$(SLO)$/xmlcnitm.obj \
		$(SLO)$/customshapeitem.obj \
        $(SLO)$/charhiddenitem.obj

.IF "$(UPD)" == "310"
LIB1OBJFILES += \
		$(SLO)$/borderline.obj
.ENDIF		# "$(UPD)" == "310"

LIB2TARGET= $(SLB)$/$(TARGET).lib
LIB2OBJFILES= \
		$(SLO)$/ofaitem.obj \
		$(SLO)$/postattr.obj	\
		$(SLO)$/hlnkitem.obj \
		$(SLO)$/pageitem.obj	\
                $(SLO)$/viewlayoutitem.obj    \
                $(SLO)$/paperinf.obj    \
		$(SLO)$/algitem.obj \
		$(SLO)$/rotmodit.obj \
		$(SLO)$/numinf.obj	\
		$(SLO)$/svxerr.obj	\
		$(SLO)$/numfmtsh.obj	\
		$(SLO)$/zoomitem.obj \
		$(SLO)$/svxempty.obj \
        $(SLO)$/SmartTagItem.obj \
        $(SLO)$/zoomslideritem.obj \

SLOFILES = $(LIB1OBJFILES) $(LIB2OBJFILES)

EXCEPTIONSFILES= \
		$(SLO)$/paraitem.obj \
		$(SLO)$/frmitems.obj \
		$(SLO)$/numitem.obj\
		$(SLO)$/xmlcnitm.obj\
		$(SLO)$/flditem.obj \
		$(SLO)$/customshapeitem.obj

.INCLUDE :	target.mk

