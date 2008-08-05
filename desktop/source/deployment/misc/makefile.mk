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
#     Modified June 2007 by Patrick Luby. NeoOffice is distributed under
#     GPL only under modification term 3 of the LGPL.
#
#*************************************************************************

PRJ = ..$/..$/..

PRJNAME = desktop
TARGET = deployment_misc
USE_DEFFILE = TRUE
ENABLE_EXCEPTIONS = TRUE

.IF "$(GUI)"=="OS2"
TARGET = deplmisc
.ENDIF

.INCLUDE : settings.mk

.IF "$(PRODUCT_DIR_NAME)" != ""
CDEFS += -DPRODUCT_DIR_NAME='"$(PRODUCT_DIR_NAME)"'
.ENDIF

# Reduction of exported symbols:
CDEFS += -DDESKTOP_DEPLOYMENTMISC_DLLIMPLEMENTATION
.IF "$(COMNAME)" == "gcc3" && "$(HAVE_GCC_VISIBILITY_FEATURE)" == "TRUE"
CFLAGS += -fvisibility=hidden
.ELIF "$(COMNAME)" == "sunpro5" && "$(CCNUMVER)" >= "00050005"
CFLAGS += -xldscope=hidden
.ENDIF

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF

SRS1NAME = $(TARGET)
SRC1FILES = \
	dp_misc.src

.IF "$(GUI)"=="OS2"
SHL1TARGET = $(TARGET)
.ELSE
SHL1TARGET = deploymentmisc$(DLLPOSTFIX)
.ENDIF
SHL1OBJS = \
        $(SLO)$/dp_misc.obj \
        $(SLO)$/dp_resource.obj \
        $(SLO)$/dp_identifier.obj \
        $(SLO)$/dp_interact.obj \
        $(SLO)$/dp_ucb.obj \
        $(SLO)$/db.obj \
        $(SLO)$/dp_version.obj \
        $(SLO)$/dp_descriptioninfoset.obj \
        $(SLO)$/dp_dependencies.obj \
        $(SLO)$/dp_platform.obj
        
SHL1STDLIBS = \
    $(BERKELEYLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(SALLIB) \
    $(TOOLSLIB) \
    $(UCBHELPERLIB) \
    $(UNOTOOLSLIB) \
    $(XMLSCRIPTLIB)
.IF "$(GUI)"=="OS2"
SHL1IMPLIB = ideploymentmisc$(DLLPOSTFIX)
LIB1TARGET = $(SLB)$/_deplmisc.lib
LIB1OBJFILES = $(SHL1OBJS)
DEFLIB1NAME = _deplmisc
.ELSE
SHL1IMPLIB = i$(SHL1TARGET)
.ENDIF
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE : ..$/target.pmk
.INCLUDE : target.mk

