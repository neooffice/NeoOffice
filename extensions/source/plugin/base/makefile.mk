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
# Modified October 2012 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************
PRJ=..$/..$/..

PRJNAME=extensions
TARGET=plbase
TARGETTYPE=GUI
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  ..$/util$/makefile.pmk

INCPRE+=-I$(SOLARINCDIR)$/mozilla$/plugin
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/java
INCPRE+=-I$(SOLARINCDIR)$/mozilla$/nspr
CDEFS+=-DOJI

.IF "$(DISABLE_XAW)" != ""
CDEFS+=-DDISABLE_XAW
.ENDIF

.IF "$(WITH_MOZILLA)" != "NO"

.IF "$(GUIBASE)"=="aqua"
OBJCXXFLAGS=-x objective-c++ -fobjc-exceptions
CFLAGSCXX+=$(OBJCXXFLAGS)
.ENDIF  # "$(GUIBASE)"=="aqua"

SLOFILES=		\
				$(SLO)$/plctrl.obj		\
				$(SLO)$/service.obj		\
				$(SLO)$/xplugin.obj		\
				$(SLO)$/nfuncs.obj		\
				$(SLO)$/manager.obj		\
				$(SLO)$/context.obj		\
				$(SLO)$/evtlstnr.obj	\
				$(SLO)$/plcom.obj		\
				$(SLO)$/multiplx.obj    \
				$(SLO)$/plmodel.obj

.ENDIF # $(WITH_MOZILLA) != "NO"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

