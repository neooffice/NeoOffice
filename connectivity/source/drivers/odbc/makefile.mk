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
# Modified August 2008 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=odbcbase
TARGET2=odbc

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
LDUMP=ldump2.exe

# --- Settings ----------------------------------

.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

.IF "$(SYSTEM_ODBC_HEADERS)" == "YES"
CFLAGS+=-DSYSTEM_ODBC_HEADERS
.ENDIF

# --- Files -------------------------------------

SLOFILES=\
		$(SLO)$/OPreparedStatement.obj			\
		$(SLO)$/OStatement.obj					\
		$(SLO)$/OResultSetMetaData.obj			\
		$(SLO)$/OResultSet.obj					\
		$(SLO)$/OTools.obj						\
		$(SLO)$/ODatabaseMetaDataResultSet.obj	\
		$(SLO)$/ODatabaseMetaData.obj			\
		$(SLO)$/ODriver.obj						\
		$(SLO)$/OConnection.obj

# --- ODBC BASE Library -----------------------------------

SHL1TARGET=	$(ODBC2_TARGET)$(ODBC2_MAJOR)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
	$(DBTOOLSLIB)				\
	$(COMPHELPERLIB)			\
	$(CPPUHELPERLIB)			\
	$(CPPULIB)					\
	$(VOSLIB)					\
	$(SALLIB)

.IF "$(GUIBASE)" == "java"
SHL1STDLIBS+=\
	-framework Carbon
.ENDIF		# "$(GUIBASE)" == "java"

SHL1DEPN=
SHL1IMPLIB=	i$(ODBC2_TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
			$(SLB)$/$(TARGET).lib
DEFLIB1NAME=$(TARGET)

# --- ODBC Library -----------------------------------
# --- Files -------------------------------------

SLO2FILES=\
		$(SLO)$/oservices.obj	\
		$(SLO)$/ORealDriver.obj	\
		$(SLO)$/OFunctions.obj

# --- ODBC Library -----------------------------------

SHL2TARGET=	$(ODBC_TARGET)$(ODBC_MAJOR)
SHL2OBJS=$(SLO2FILES)
SHL2STDLIBS=\
	$(ODBCBASELIB)				\
	$(CPPUHELPERLIB)			\
	$(CPPULIB)					\
	$(SALLIB)

.IF "$(ODBCBASELIB)" == ""
SHL2STDLIBS+= $(ODBCBASELIB)
.ENDIF

SHL2DEPN=$(SHL1TARGETN)
SHL2IMPLIB=	i$(ODBC_TARGET)

SHL2DEF=	$(MISC)$/$(SHL2TARGET).def

DEF2NAME=	$(SHL2TARGET)
SHL2VERSIONMAP=odbc.map

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- filter file ------------------------------

.IF "$(depend)"==""

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
	@echo ------------------------------
    @echo CLEAR_THE_FILE	> $@
	@echo _TI				>>$@
	@echo _real				>>$@
.ENDIF

