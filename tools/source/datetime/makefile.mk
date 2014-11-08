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

PRJNAME=tools
TARGET=datetime

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :      $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=   $(SLO)$/tdate.obj     \
            $(SLO)$/ttime.obj     \
            $(SLO)$/datetime.obj

OBJFILES=   $(OBJ)$/tdate.obj     \
            $(OBJ)$/ttime.obj     \
            $(OBJ)$/datetime.obj

.IF "$(UPD)" == "310"
SLOFILES += \
	$(SLO)$/datetimeutils.obj

OBJFILES += \
	$(OBJ)$/datetimeutils.obj
.ENDIF		# "$(UPD)" == "310"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
