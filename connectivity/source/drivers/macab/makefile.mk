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
# Modified September 2007 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=macab
TARGET2=$(TARGET)drv

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(GUI)" == "UNX"
.IF "$(OS)" == "MACOSX"

# === MACAB base library ==========================

# --- Files -------------------------------------

SLOFILES= \
    $(SLO)$/MacabDriver.obj     \
    $(SLO)$/MacabServices.obj

DEPOBJFILES= \
	$(SLO2FILES)

# --- Library -----------------------------------

SHL1VERSIONMAP=$(TARGET).map

SHL1TARGET= $(TARGET)$(MACAB_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
    $(CPPULIB)                  \
    $(CPPUHELPERLIB)            \
    $(DBTOOLSLIB)               \
    $(SALLIB)

SHL1DEPN=
SHL1IMPLIB= i$(TARGET)

SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME=   $(SHL1TARGET)

# === MACAB impl library ==========================

# --- Files -------------------------------------

SLO2FILES=\
    $(SLO)$/MacabColumns.obj            \
    $(SLO)$/MacabTable.obj              \
    $(SLO)$/MacabTables.obj             \
    $(SLO)$/MacabCatalog.obj            \
    $(SLO)$/MacabResultSet.obj          \
    $(SLO)$/MacabStatement.obj          \
    $(SLO)$/MacabPreparedStatement.obj  \
    $(SLO)$/MacabDatabaseMetaData.obj   \
    $(SLO)$/MacabConnection.obj         \
    $(SLO)$/MacabResultSetMetaData.obj  \
    $(SLO)$/macabcondition.obj          \
    $(SLO)$/macaborder.obj              \
		$(SLO)$/MacabRecord.obj             \
		$(SLO)$/MacabRecords.obj            \
		$(SLO)$/MacabHeader.obj             \
		$(SLO)$/MacabGroup.obj              \
		$(SLO)$/MacabAddressBook.obj

.IF "$(GUIBASE)"=="java" 
MACAB_LIB=-framework CoreFoundation -framework AddressBook
.ELSE		# "$(GUIBASE)"=="java" 
MACAB_LIB=-framework Carbon -framework AddressBook
.ENDIF		# "$(GUIBASE)"=="java" 

# --- Library -----------------------------------

SHL2VERSIONMAP=$(TARGET2).map

SHL2TARGET= $(TARGET2)$(MACAB_MAJOR)
SHL2OBJS=$(SLO2FILES)
SHL2STDLIBS=\
    $(CPPULIB)                  \
    $(CPPUHELPERLIB)            \
    $(VOSLIB)                   \
    $(SALLIB)                   \
    $(DBTOOLSLIB)               \
    $(COMPHELPERLIB)            \
    $(MACAB_LIB)

SHL2DEPN=
SHL2IMPLIB= i$(TARGET2)

SHL2DEF=    $(MISC)$/$(SHL2TARGET).def

DEF2NAME=   $(SHL2TARGET)

# --- Targets -----------------------------------
.ELSE		# "$(OS)" == "MACOSX"
dummy:
	@echo Not using Mac OS X - nothing to build
.ENDIF

.ELSE		# "$(GUI)" == "UNX"
dummy:
	@echo "Nothing to build for GUI $(GUI)"
.ENDIF

.INCLUDE : target.mk

