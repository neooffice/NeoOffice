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
# Modified December 2016 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=sw
TARGET=app

LIBTARGET=NO

# future: DEMO\...

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

.IF "$(UPD)" == "310"
INCLOCAL += \
	-I$(PRJ)$/..$/comphelper$/inc
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =\
        app.src     \
        mn.src      \
        error.src


SLOFILES = \
        $(SLO)$/appenv.obj   \
        $(SLO)$/apphdl.obj   \
        $(SLO)$/applab.obj   \
		$(SLO)$/appopt.obj   \
		$(SLO)$/docsh.obj    \
		$(SLO)$/docsh2.obj   \
		$(SLO)$/docshdrw.obj \
		$(SLO)$/docshini.obj \
		$(SLO)$/docst.obj    \
		$(SLO)$/docstyle.obj \
		$(SLO)$/mainwn.obj   \
		$(SLO)$/swmodule.obj \
		$(SLO)$/swmodul1.obj \
		$(SLO)$/swdll.obj	 \
		$(SLO)$/swwait.obj

EXCEPTIONSFILES= \
		$(SLO)$/docsh.obj    \
		$(SLO)$/docst.obj    \
		$(SLO)$/swmodule.obj \
		$(SLO)$/swmodul1.obj \
        	$(SLO)$/apphdl.obj   \
		$(SLO)$/docsh2.obj

LIB1TARGET= $(SLB)$/app.lib

LIB1OBJFILES= \
        $(SLO)$/appenv.obj   \
        $(SLO)$/apphdl.obj   \
        $(SLO)$/applab.obj   \
		$(SLO)$/appopt.obj   \
		$(SLO)$/docsh.obj    \
		$(SLO)$/docsh2.obj   \
		$(SLO)$/docshdrw.obj \
		$(SLO)$/docshini.obj \
		$(SLO)$/docst.obj    \
		$(SLO)$/docstyle.obj \
		$(SLO)$/mainwn.obj   \
		$(SLO)$/swmodul1.obj \
		$(SLO)$/swwait.obj


# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(SRS)$/app.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc

