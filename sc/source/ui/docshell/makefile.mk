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
# Modified July 2015 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=sc
TARGET=docshell
LIBTARGET=no

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(UPD)" == "310"
INCLOCAL += \
	-I$(PRJ)$/..$/svtools$/inc
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

CXXFILES = \
        docsh.cxx	\
        docsh2.cxx	\
        docsh3.cxx	\
        docsh4.cxx 	\
        docsh5.cxx	\
        docsh6.cxx 	\
        docsh7.cxx 	\
        docsh8.cxx 	\
        externalrefmgr.cxx \
        tablink.cxx 	\
        arealink.cxx 	\
        dbdocfun.cxx 	\
        dbdocimp.cxx 	\
        impex.cxx	\
        docfunc.cxx	\
        olinefun.cxx	\
        servobj.cxx	\
		tpstat.cxx	\
		autostyl.cxx	\
		pagedata.cxx \
		hiranges.cxx \
		pntlock.cxx \
		sizedev.cxx \
		editable.cxx


SLOFILES =  \
        $(SLO)$/docsh.obj	\
        $(SLO)$/docsh2.obj	\
        $(SLO)$/docsh3.obj	\
        $(SLO)$/docsh4.obj   	\
        $(SLO)$/docsh5.obj   	\
        $(SLO)$/docsh6.obj   	\
        $(SLO)$/docsh7.obj   	\
        $(SLO)$/docsh8.obj   	\
        $(SLO)$/externalrefmgr.obj \
        $(SLO)$/tablink.obj   	\
        $(SLO)$/arealink.obj   	\
        $(SLO)$/dbdocfun.obj 	\
        $(SLO)$/dbdocimp.obj 	\
        $(SLO)$/impex.obj	\
        $(SLO)$/docfunc.obj	\
        $(SLO)$/olinefun.obj	\
        $(SLO)$/servobj.obj	\
        $(SLO)$/tpstat.obj	\
        $(SLO)$/autostyl.obj	\
        $(SLO)$/pagedata.obj \
        $(SLO)$/hiranges.obj \
        $(SLO)$/pntlock.obj \
        $(SLO)$/sizedev.obj \
        $(SLO)$/editable.obj \
	$(SLO)$/macromgr.obj


EXCEPTIONSFILES= \
        $(SLO)$/docsh.obj \
        $(SLO)$/docsh3.obj	\
        $(SLO)$/docsh4.obj \
        $(SLO)$/docsh8.obj \
        $(SLO)$/externalrefmgr.obj \
        $(SLO)$/dbdocimp.obj \
        $(SLO)$/docfunc.obj \
	$(SLO)$/macromgr.obj

SRS1NAME=$(TARGET)
SRC1FILES =  tpstat.src

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =  \
        $(SLO)$/docsh.obj	\
        $(SLO)$/docsh2.obj	\
        $(SLO)$/docsh3.obj	\
        $(SLO)$/docsh4.obj   	\
        $(SLO)$/docsh5.obj   	\
        $(SLO)$/docsh6.obj   	\
        $(SLO)$/docsh7.obj   	\
        $(SLO)$/docsh8.obj   	\
        $(SLO)$/externalrefmgr.obj \
        $(SLO)$/tablink.obj   	\
        $(SLO)$/arealink.obj   	\
        $(SLO)$/dbdocfun.obj 	\
        $(SLO)$/dbdocimp.obj 	\
        $(SLO)$/impex.obj	\
        $(SLO)$/docfunc.obj	\
        $(SLO)$/olinefun.obj	\
        $(SLO)$/servobj.obj	\
        $(SLO)$/autostyl.obj	\
        $(SLO)$/pagedata.obj \
        $(SLO)$/hiranges.obj \
        $(SLO)$/pntlock.obj \
        $(SLO)$/sizedev.obj \
        $(SLO)$/editable.obj \
	$(SLO)$/macromgr.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

