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

PROJECTPCH=sd
PROJECTPCHSOURCE=$(PRJ)$/util$/sd
PRJNAME=sd
TARGET=core
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

IENV+=-I..\ui\inc

.IF "$(UPD)" == "310"
INCLOCAL += \
	-I$(PRJ)$/..$/comphelper$/inc
.ENDIF		# "$(UPD)" == "310"

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES = glob.src

SLOFILES = $(SLO)$/stlsheet.obj  \
		   $(SLO)$/stlfamily.obj \
		   $(SLO)$/stlpool.obj	\
		   $(SLO)$/drawdoc.obj \
		   $(SLO)$/drawdoc2.obj \
           $(SLO)$/drawdoc3.obj \
		   $(SLO)$/drawdoc4.obj \
		   $(SLO)$/drawdoc_animations.obj\
		   $(SLO)$/sdpage.obj \
		   $(SLO)$/sdpage2.obj	\
		   $(SLO)$/sdattr.obj \
		   $(SLO)$/sdobjfac.obj \
		   $(SLO)$/anminfo.obj	\
		   $(SLO)$/sdiocmpt.obj	\
		   $(SLO)$/typemap.obj	\
		   $(SLO)$/pglink.obj   \
           $(SLO)$/cusshow.obj  \
           $(SLO)$/PageListWatcher.obj  \
           $(SLO)$/sdpage_animations.obj\
           $(SLO)$/CustomAnimationPreset.obj\
           $(SLO)$/CustomAnimationEffect.obj\
           $(SLO)$/TransitionPreset.obj\
           $(SLO)$/undoanim.obj\
           $(SLO)$/EffectMigration.obj\
           $(SLO)$/CustomAnimationCloner.obj\
           $(SLO)$/shapelist.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

