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
# Modified November 2007 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=.

PRJNAME=ooo_rhino
TARGET=ooo_rhino

.IF "$(SOLAR_JAVA)"!=""
# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  antsettings.mk

# --- Files --------------------------------------------------------

TARFILE_NAME=rhino1_5R5
TARFILE_ROOTDIR=rhino1_5R5

ADDITIONAL_FILES= \
	toolsrc/org/mozilla/javascript/tools/debugger/OfficeScriptInfo.java

PATCH_FILE_NAME=rhino1_5R5.patch

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj jar
.ELSE
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dant.build.javac.source=$(JAVA_SOURCE_VER) -Dant.build.javac.target=$(JAVA_TARGET_VER) jar
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.IF "$(GUIBASE)" == "java"
BACK_PATH:=$(BACK_PATH)..$/..$/rhino$/
.ENDIF		# "$(GUIBASE)" == "java"

.ELSE
all:
        @echo java disabled
.ENDIF
