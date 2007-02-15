#*************************************************************************
#
#   $RCSfile$
#
#   $Revision$
#
#   last change: $Author$ $Date$
#
#   The Contents of this file are made available subject to
#   the terms of GNU General Public License Version 2.1.
#
#
#     GNU General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     General Public License for more details.
#
#     You should have received a copy of the GNU General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#     Modified February 2007 by Patrick Luby. NeoOffice is distributed under
#     GPL only under modification term 3 of the LGPL.
#
#*************************************************************************

PRJ=.

PRJNAME=so_hsqldb
TARGET=so_hsqldb

.IF "$(SOLAR_JAVA)" != ""
# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  version.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=hsqldb_$(HSQLDB_VERSION)

TARFILE_ROOTDIR=hsqldb

CONVERTFILES=build$/build.xml\
		doc/changelist_1_8_0.txt\
		src/org/hsqldb/resources/sql-error-messages.properties
PATCH_FILE_NAME=hsqldb_1_8_0

# We need this hack in our build system because the OOo build rules will
# only see the unmodified OOo patch file and not our replacement patch file
GNUPATCH:=$(TYPE) /dev/stdin $(PWD)$/$(PATCH_FILE_NAME) | $(GNUPATCH)

ADDITIONAL_FILES=makefile.mk

# ADDITIONAL_FILES=   src$/org$/hsqldb$/Collation.java \
#                     src$/org$/hsqldb$/TxManager.java \
#                     src$/org$/hsqldb$/lib$/LongKeyIntValueHashMap.java \
#                     src$/org$/hsqldb$/persist$/ScaledRAFileInJar.java \
#                     src$/org$/hsqldb$/test$/TestCollation.java

BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.ELSE
all:
        @echo java disabled
.ENDIF
