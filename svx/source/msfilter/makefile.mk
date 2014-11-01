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
# Modified October 2014 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=svx
TARGET=msfilter
LIBTARGET=NO
AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(UPD)" == "310"
INCLOCAL += -I$(PRJ)$/..$/sal/inc
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET)-msfilter.lib
LIB1OBJFILES= \
	$(SLO)$/countryid.obj	\
	$(SLO)$/escherex.obj	\
	$(SLO)$/eschesdo.obj    \
	$(SLO)$/msdffimp.obj	\
	$(SLO)$/msoleexp.obj	\
	$(SLO)$/msvbasic.obj	\
	$(SLO)$/msashape.obj	\
	$(SLO)$/svxmsbas.obj	\
	$(SLO)$/msocximex.obj	\
	$(SLO)$/msashape3d.obj	\
	$(SLO)$/mscodec.obj		\
	$(SLO)$/msfiltertracer.obj\
	$(SLO)$/mstoolbar.obj\
	$(SLO)$/msvbahelper.obj\

LIB2TARGET= $(SLB)$/$(TARGET)-core.lib
LIB2OBJFILES= \
	$(SLO)$/svxmsbas2.obj

SLOFILES = $(LIB1OBJFILES) $(LIB2OBJFILES)

EXCEPTIONSFILES= \
	$(SLO)$/eschesdo.obj	\
	$(SLO)$/escherex.obj	\
	$(SLO)$/msdffimp.obj	\
	$(SLO)$/msashape3d.obj	\
	$(SLO)$/msvbasic.obj	\
	$(SLO)$/msocximex.obj	\
	$(SLO)$/msoleexp.obj	\
	$(SLO)$/svxmsbas.obj	\
	$(SLO)$/msfiltertracer.obj\
	$(SLO)$/mstoolbar.obj\
	$(SLO)$/msvbahelper.obj\

.INCLUDE :  target.mk

