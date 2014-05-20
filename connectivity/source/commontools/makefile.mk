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
# Modified May 2014 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=connectivity
TARGET=commontools

# --- Settings -----------------------------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/dbtools.pmk

# Disable optimization for SunCC SPARC and MACOSX (funny loops
# when parsing e.g. "x+width/2"),
# also http://gcc.gnu.org/PR22392
.IF ("$(OS)$(CPU)"=="SOLARISS" && "$(COM)"!="GCC") || "$(OS)"=="MACOSX" || ("$(OS)"=="LINUX" && "$(CPU)"=="P") 
NOOPTFILES= $(SLO)$/RowFunctionParser.obj
.ENDIF

.IF "$(GUIBASE)" == "java" && "$(OS)" == "MACOSX"
# Fix infinite looping bug reported in the following NeoOffice forum post by
# turning off optimization for all source files:
# http://trinity.neooffice.org/modules.php?name=Forums&file=viewtopic&p=64787#64787
CDEFS+=-O0
.ENDIF

# --- Files --------------------------------------------------------
EXCEPTIONSFILES=\
		$(SLO)$/predicateinput.obj						\
		$(SLO)$/ConnectionWrapper.obj					\
		$(SLO)$/TConnection.obj							\
		$(SLO)$/conncleanup.obj							\
		$(SLO)$/dbtools.obj								\
		$(SLO)$/dbtools2.obj							\
		$(SLO)$/dbexception.obj							\
		$(SLO)$/CommonTools.obj							\
		$(SLO)$/TColumnsHelper.obj						\
		$(SLO)$/TTableHelper.obj						\
		$(SLO)$/TKeys.obj								\
		$(SLO)$/TKey.obj								\
		$(SLO)$/TKeyColumns.obj							\
		$(SLO)$/TIndexes.obj							\
		$(SLO)$/TIndex.obj								\
		$(SLO)$/TIndexColumns.obj						\
		$(SLO)$/DateConversion.obj						\
		$(SLO)$/FDatabaseMetaDataResultSetMetaData.obj	\
		$(SLO)$/FDatabaseMetaDataResultSet.obj			\
		$(SLO)$/TDatabaseMetaDataBase.obj				\
		$(SLO)$/TPrivilegesResultSet.obj				\
		$(SLO)$/TSkipDeletedSet.obj                     \
		$(SLO)$/dbmetadata.obj                          \
        $(SLO)$/TSortIndex.obj                          \
        $(SLO)$/dbcharset.obj                           \
        $(SLO)$/propertyids.obj                         \
        $(SLO)$/FValue.obj                              \
        $(SLO)$/paramwrapper.obj                        \
        $(SLO)$/statementcomposer.obj                   \
        $(SLO)$/RowFunctionParser.obj                   \
        $(SLO)$/sqlerror.obj                            \
        $(SLO)$/filtermanager.obj                       \
        $(SLO)$/parameters.obj							\
        $(SLO)$/ParamterSubstitution.obj                \
        $(SLO)$/formattedcolumnvalue.obj

SLOFILES=\
		$(EXCEPTIONSFILES)								\
		$(SLO)$/AutoRetrievingBase.obj					\
		$(SLO)$/dbconversion.obj




# --- Targets ------------------------------------------------------

.INCLUDE :      target.mk


