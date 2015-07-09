#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.
#  
#  $RCSfile$
#  $Revision$
#  
#  This file is part of NeoOffice.
#  
#  NeoOffice is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 3
#  only, as published by the Free Software Foundation.
#  
#  NeoOffice is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License version 3 for more details
#  (a copy is included in the LICENSE file that accompanied this code).
#  
#  You should have received a copy of the GNU General Public License
#  version 3 along with NeoOffice.  If not, see
#  <http://www.gnu.org/licenses/gpl-3.0.txt>
#  for a copy of the GPLv3 License.
#  
#  Modified July 2015 by Patrick Luby. NeoOffice is distributed under
#  GPL only under Section 4 of the Apache License v2.0.
#  
#**************************************************************



PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=adabas

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.IF "$(UPD)" == "310"
.INCLUDE : settings.mk
.ELSE		# "$(UPD)" == "310"
.INCLUDE : $(PRJ)$/makefile.pmk
.ENDIF		# "$(UPD)" == "310"
.INCLUDE : $(PRJ)$/version.mk

.IF "$(SYSTEM_ODBC_HEADERS)" == "YES"
CFLAGS+=-DSYSTEM_ODBC_HEADERS
.ENDIF

.IF "$(UPD)" == "310"
INCLOCAL+= \
	-I$(PRJ)$/..$/sal$/inc
.ENDIF		# "$(UPD)" == "310"

# --- Files -------------------------------------

SLOFILES=\
		$(SLO)$/BFunctions.obj					\
		$(SLO)$/BConnection.obj					\
		$(SLO)$/BDriver.obj						\
		$(SLO)$/BCatalog.obj					\
		$(SLO)$/BGroups.obj						\
		$(SLO)$/BGroup.obj						\
		$(SLO)$/BUser.obj						\
		$(SLO)$/BUsers.obj						\
		$(SLO)$/BKeys.obj						\
		$(SLO)$/BColumns.obj					\
		$(SLO)$/BIndex.obj						\
		$(SLO)$/BIndexColumns.obj				\
		$(SLO)$/BIndexes.obj					\
		$(SLO)$/BTable.obj						\
		$(SLO)$/BTables.obj						\
		$(SLO)$/BViews.obj						\
		$(SLO)$/Bservices.obj					\
		$(SLO)$/BDatabaseMetaData.obj			\
        $(SLO)$/BPreparedStatement.obj          \
        $(SLO)$/BStatement.obj                  \
		$(SLO)$/BResultSetMetaData.obj			\
        $(SLO)$/BResultSet.obj

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Library -----------------------------------

SHL1TARGET=	$(TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
	$(CPPULIB)					\
	$(CPPUHELPERLIB)			\
	$(VOSLIB)					\
	$(SALLIB)					\
	$(DBTOOLSLIB)				\
	$(TOOLSLIB)					\
	$(ODBCBASELIB)				\
	$(UNOTOOLSLIB)				\
	$(COMPHELPERLIB)

.IF "$(ODBCBASELIB)" == ""
SHL1STDLIBS+=$(ODBCBASELIB)
.ENDIF

SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ----------------------------------


.IF "$(UPD)" == "310"

.INCLUDE : target.mk

.ELSE		# "$(UPD)" == "310"

.INCLUDE : $(PRJ)$/target.pmk

ALLTAR : $(MISC)/adabas.component

$(MISC)/adabas.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        adabas.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt adabas.component

.ENDIF		# "$(UPD)" == "310"
