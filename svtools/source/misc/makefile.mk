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

PRJ=..$/..
PRJNAME=svtools
TARGET=misc

ENABLE_EXCEPTIONS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/svt.pmk

.IF "$(UPD)" == "310"
INCLOCAL += \
	-I$(PRJ)$/..$/comphelper$/inc
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

#use local "bmp" as it may not yet be delivered

SRS1NAME=misc
SRC1FILES=\
	config.src	\
	iniman.src 	\
	ehdl.src \
	imagemgr.src      \
    helpagent.src \
    langtab.src 

SRS2NAME=ehdl
SRC2FILES=\
	errtxt.src

SLOFILES=\
    $(SLO)$/acceleratorexecute.obj  \
    $(SLO)$/cliplistener.obj        \
    $(SLO)$/embedhlp.obj            \
    $(SLO)$/embedtransfer.obj       \
    $(SLO)$/imagemgr.obj            \
    $(SLO)$/imageresourceaccess.obj \
    $(SLO)$/templatefoldercache.obj \
    $(SLO)$/transfer.obj            \
    $(SLO)$/transfer2.obj           \
    $(SLO)$/stringtransfer.obj      \
    $(SLO)$/urihelper.obj           \
    $(SLO)$/svtaccessiblefactory.obj \
    $(SLO)$/ehdl.obj                \
    $(SLO)$/flbytes.obj             \
    $(SLO)$/helpagentwindow.obj     \
    $(SLO)$/imap.obj                \
    $(SLO)$/imap2.obj               \
    $(SLO)$/imap3.obj               \
    $(SLO)$/ownlist.obj             \
    $(SLO)$/vcldata.obj             \
    $(SLO)$/restrictedpaths.obj     \
    $(SLO)$/dialogclosedlistener.obj\
    $(SLO)$/dialogcontrolling.obj   \
    $(SLO)$/chartprettypainter.obj \
    $(SLO)$/lockfilecommon.obj     \
    $(SLO)$/sharecontrolfile.obj   \
    $(SLO)$/documentlockfile.obj   \
    $(SLO)$/bindablecontrolhelper.obj   \
    $(SLO)$/filterutils.obj   \
    $(SLO)$/langtab.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

