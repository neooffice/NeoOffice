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

PRJ=..$/..

PRJNAME=svtools
TARGET=java
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/svt.pmk

.IF "$(PRODUCT_JAVA_DOWNLOAD_URL)" != ""
CDEFS += -DPRODUCT_JAVA_DOWNLOAD_URL='"$(PRODUCT_JAVA_DOWNLOAD_URL)"'
.ENDIF		# "$(PRODUCT_JAVA_DOWNLOAD_URL)" != ""

# --- Files --------------------------------------------------------

SRS1NAME= javaerror
SRC1FILES= javaerror.src

SRS2NAME= patchjavaerror
SRC2FILES= patchjavaerror.src	


SLOFILES= \
	$(SLO)$/javainteractionhandler.obj \
	$(SLO)$/javacontext.obj

.IF "$(GUIBASE)" == "java"
SLOFILES += \
	$(SLO)$/javainteractionhandler_cocoa.obj
.ENDIF		# "$(GUIBASE)" == "java"

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
