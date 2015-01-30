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
# Modified January 2015 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=				..$/..$/..
PRJINC=				$(PRJ)$/source
PRJNAME=			chart2
TARGET=				chvaxes

ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/chartview.pmk

#.IF "$(GUI)" == "WNT"
#CFLAGS+=-GR
#.ENDIF

.IF "$(UPD)" == "310"
.INCLUDE: $(PRJ)$/inc$/chart2.mk
.ENDIF		# "$(UPD)" == "310"

# --- export library -------------------------------------------------

#object files to build and link together to lib $(SLB)$/$(TARGET).lib
SLOFILES = \
    $(SLO)$/VAxisOrGridBase.obj \
    $(SLO)$/VAxisBase.obj \
    $(SLO)$/TickmarkHelper.obj \
    $(SLO)$/MinimumAndMaximumSupplier.obj \
    $(SLO)$/ScaleAutomatism.obj \
    $(SLO)$/VAxisProperties.obj \
    $(SLO)$/VCartesianAxis.obj \
    $(SLO)$/VCartesianGrid.obj \
    $(SLO)$/VCartesianCoordinateSystem.obj \
    $(SLO)$/VPolarAxis.obj \
    $(SLO)$/VPolarAngleAxis.obj \
    $(SLO)$/VPolarRadiusAxis.obj \
    $(SLO)$/VPolarGrid.obj \
    $(SLO)$/VPolarCoordinateSystem.obj \
    $(SLO)$/VCoordinateSystem.obj

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk
