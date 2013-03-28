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
# Modified March 2013 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

# 2 == Unicode
MAJOR_VERSION=2

PRJ=..
PRJNAME=package
TARGET=package

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- General ----------------------------------------------------

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	\
    $(SLB)$/zipapi.lib \
    $(SLB)$/zippackage.lib \
    $(SLB)$/manifest.lib

# --- Shared-Library -----------------------------------------------

SHL1TARGET=$(TARGET)$(MAJOR_VERSION)
SHL1IMPLIB=i$(TARGET)
SHL1VERSIONMAP=$(SOLARENV)$/src$/component.map

SHL1STDLIBS=\
	$(CPPULIB)		\
	$(UCBHELPERLIB)		\
	$(CPPUHELPERLIB)	\
	$(COMPHELPERLIB)		\
	$(SALLIB)		\
	$(ZLIB3RDLIB)

.IF "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"
SHL1STDLIBS += $(NSPR4LIB) $(NSS3LIB)
.ENDIF		# "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)
DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------------------------------

.INCLUDE :  target.mk

