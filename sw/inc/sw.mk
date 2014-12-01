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

# CDEFS=$(CDEFS) -DXXX

# Bereiche aktivieren

#CDEFS+=-DXML_CORE_API
CDEFS+=-DACCESSIBLE_LAYOUT

# define SW_DLLIMPLEMENTATION (see @ swdllapi.h)
.IF "$(MAKING_LIBMSWORD)" != "TRUE"
CDEFS += -DSW_DLLIMPLEMENTATION
.ENDIF

VISIBILITY_HIDDEN=TRUE

.IF "$(UPD)" == "310"
# Avoid conflicting header file names by putting this project first in the list
INCLOCAL += \
	-I$(PRJ)$/inc \
	-I$(PRJ)$/..$/comphelper$/inc \
	-I$(PRJ)$/..$/offapi$/$(INPATH)$/inc$/csstext \
	-I$(PRJ)$/..$/oox$/inc \
	-I$(PRJ)$/..$/oox$/$(INPATH)$/inc \
	-I$(PRJ)$/..$/sal$/inc \
	-I$(PRJ)$/..$/sax$/inc \
	-I$(PRJ)$/..$/sfx2$/inc \
	-I$(PRJ)$/..$/svtools$/inc \
	-I$(PRJ)$/..$/svx$/inc
.ENDIF		# "$(UPD)" == "310"
