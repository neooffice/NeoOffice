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
# Modified December 2008 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=.

PRJNAME=hsqldb
TARGET=so_hsqldb

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# override buildfile
ANT_BUILDFILE=build$/build.xml

.INCLUDE : antsettings.mk

.INCLUDE :  version.mk

.IF "$(SOLAR_JAVA)" != ""
# --- Files --------------------------------------------------------

TARFILE_NAME=hsqldb_$(HSQLDB_VERSION)

TARFILE_ROOTDIR=hsqldb

CONVERTFILES=build$/build.xml

PATCH_FILE_NAME=patches$/accumulated_patches.patch

# ADDITIONAL_FILES=   src$/org$/hsqldb$/Collation.java \
#                     src$/org$/hsqldb$/TxManager.java \
#                     src$/org$/hsqldb$/lib$/LongKeyIntValueHashMap.java \
#                     src$/org$/hsqldb$/persist$/ScaledRAFileInJar.java \
#                     src$/org$/hsqldb$/test$/TestCollation.java

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj -f $(ANT_BUILDFILE) jar
.ELSE
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dant.build.javac.source=$(JAVA_SOURCE_VER) -Dant.build.javac.target=$(JAVA_TARGET_VER) -f $(ANT_BUILDFILE) jar
.ENDIF

.ENDIF # $(SOLAR_JAVA)!= ""

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk
.ENDIF

.IF "$(GUIBASE)" == "java"
BACK_PATH:=$(BACK_PATH)..$/..$/$(PRJNAME)$/
.ENDIF		# "$(GUIBASE)" == "java"
