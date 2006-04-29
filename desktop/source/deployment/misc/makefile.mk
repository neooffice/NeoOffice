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
#     Modified April 2006 by Patrick Luby. NeoOffice is distributed under
#     GPL only under modification term 3 of the LGPL.
#
#*************************************************************************

PRJ = ..$/..$/..

PRJNAME = desktop
TARGET = deployment_misc
ENABLE_EXCEPTIONS = TRUE

.INCLUDE : settings.mk

.IF "$(PRODUCT_DIR_NAME)" != ""
CDEFS += -DPRODUCT_DIR_NAME='"$(PRODUCT_DIR_NAME)"'
.ENDIF

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF

SRS1NAME = $(TARGET)
SRC1FILES = \
	dp_misc.src

SLOFILES = \
        $(SLO)$/dp_misc.obj \
        $(SLO)$/dp_resource.obj \
        $(SLO)$/dp_interact.obj \
        $(SLO)$/dp_ucb.obj \
        $(SLO)$/dp_xml.obj \
        $(SLO)$/dp_log.obj \
        $(SLO)$/dp_persmap.obj \
        $(SLO)$/dp_servicefactory.obj \
        $(SLO)$/dp_services.obj       \
        $(SLO)$/db.obj

.INCLUDE : ..$/target.pmk
.INCLUDE : target.mk

