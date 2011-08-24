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

PRJNAME=wpd
TARGET=wpd

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_LIBWPD)" == "YES"
@all:
	@echo "Using system libwpd..."
.ENDIF

TARFILE_NAME=libwpd-0.8.14
.IF "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"
PATCH_FILE_NAME=$(TARFILE_NAME).diff
.ELSE		# "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"
#PATCH_FILE_NAME=$(TARFILE_NAME).diff
.ENDIF		# "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"
BUILD_ACTION=dmake $(MFLAGS) $(CALLMACROS)
BUILD_DIR=src$/lib

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.IF "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"
BACK_PATH:=$(PWD)$/
.ENDIF		# "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"
