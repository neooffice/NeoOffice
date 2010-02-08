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
# Modified February 2010 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=sw
TARGET=docvw
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/sw.mk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
		access.src \
		docvw.src

SLOFILES =  \
		$(SLO)$/edtdd.obj \
		$(SLO)$/edtwin.obj \
		$(SLO)$/edtwin2.obj \
		$(SLO)$/edtwin3.obj \
		$(SLO)$/romenu.obj \
		$(SLO)$/srcedtw.obj \
		$(SLO)$/postit.obj \
		$(SLO)$/PostItMgr.obj 

.IF "$(GUIBASE)" == "java"

SLOFILES += $(SLO)$/macdictlookup.obj 

SRS2NAME=macdictlookup
SRC2FILES = macdictlookup.src
RESLIB2NAME=$(PRJNAME)macdictlookup
RESLIB2IMAGES=$(PRJ)$/res
RESLIB2SRSFILES=$(SRS)$/macdictlookup.srs

.ENDIF		# "$(GUIBASE)" == "java"

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

