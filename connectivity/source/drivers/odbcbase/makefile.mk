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
#  Modified May 2014 by Patrick Luby. NeoOffice is distributed under
#  GPL only under Section 4 of the Apache License v2.0.
#  
#**************************************************************



PRJ=..$/..$/..
PRJINC=..$/..
PRJNAME=connectivity
TARGET=odbcbase

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk

CDEFS += -DOOO_DLLIMPLEMENTATION_ODBCBASE

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

SHL1TARGET=	$(ODBC2_TARGET)$(DLLPOSTFIX)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS=\
	$(DBTOOLSLIB)				\
	$(COMPHELPERLIB)			\
	$(CPPUHELPERLIB)			\
	$(CPPULIB)					\
	$(VOSLIB)					\
	$(SALLIB)

.IF "$(GUIBASE)" == "java"
SHL1STDLIBS+=-framework CoreFoundation
.ENDIF		# "$(GUIBASE)" == "java"

SHL1DEPN=
SHL1IMPLIB=	i$(ODBC2_TARGET)
SHL1USE_EXPORTS=name

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)
DEF1DEPN=	$(MISC)$/$(SHL1TARGET).flt \
			$(SLB)$/$(TARGET).lib
DEFLIB1NAME=$(TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- filter file ------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo CLEAR_THE_FILE	> $@
