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
# Modified April 2012 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=vcl
TARGET=app
ENABLE_EXCEPTIONS=TRUE

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

CDEFS+=-DDLLPOSTFIX=$(DLLPOSTFIX)

.IF "$(GUIBASE)" == "java"
CDEFS+=-DPRODUCT_MD5="$(shell md5 -q -s '$(PRODUCT_NAME)_org.neooffice.$(PRODUCT_DIR_NAME)')"
.ENDIF		# "$(GUIBASE)" == "java"

# --- Files --------------------------------------------------------

SLOFILES=	$(SLO)$/dbggui.obj		\
			$(SLO)$/help.obj		\
			$(SLO)$/idlemgr.obj 	\
			$(SLO)$/settings.obj	\
			$(SLO)$/sound.obj		\
			$(SLO)$/stdtext.obj 	\
			$(SLO)$/svapp.obj		\
			$(SLO)$/svdata.obj		\
			$(SLO)$/svmain.obj		\
			$(SLO)$/svmainhook.obj	\
			$(SLO)$/timer.obj		\
			$(SLO)$/dndhelp.obj     \
			$(SLO)$/unohelp.obj     \
			$(SLO)$/unohelp2.obj    \
			$(SLO)$/vclevent.obj	\
			$(SLO)$/i18nhelp.obj	\
			$(SLO)$/salvtables.obj	\
			$(SLO)$/session.obj

.IF "$(GUIBASE)" == "java"
SLOFILES += $(SLO)$/svmainhook_cocoa.obj
.ENDIF		# "$(GUIBASE)" == "java"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.INCLUDE :	$(PRJ)$/util$/target.pmk

