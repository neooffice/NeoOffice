##########################################################################
#
#   $RCSfile$
#
#   $Revision$
#
#   last change: $Author$ $Date$
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU General Public License Version 2.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#   
#   =================================================
#   Modified June 2004 by Patrick Luby. SISSL Removed. NeoOffice is
#   distributed under GPL only under modification term 3 of the LGPL.
#
#   Contributor(s): _______________________________________
#
##########################################################################

PRJ=..$/..$/..

PRJNAME=offmgr
TARGET=intro

# --- Settings ------------------------------------------------------------


.INCLUDE :  settings.mk

RSCLOCINC!:=$(RSCLOCINC);$(PRJ)$/RES

# setting the compiled by $user$ using $license$ string in the about box

.IF "$(BUILD_SPECIAL)"==""
RSCUPDVERMAC+=-DOOO_VENDOR="$(USER)$(USERNAME)" -DOOO_LICENSE="LGPL"
.ELSE
RSCUPDVERMAC+=-DBUILD_SPECIAL=True
.ENDIF


.INCLUDE :  $(UPD)minor.mk
RSCUPDVER=$(RSCREVISION)

#RSCUPDVER=$(_SDT_REVISION)(SV$(UPD)$(UPDMINOR))
IMGLST_SRS=$(SRS)$/iso.srs $(SRS)$/ooo.srs

# --- Allgemein -----------------------------------------------------------

SRS1NAME= iso
SRC1FILES=	\
	$(SRS1NAME).src

RESLIB1NAME=$(SRS1NAME)
RESLIB1SRSFILES= \
	$(SRS)$/$(SRS1NAME).srs

SRS2NAME= oem
SRC2FILES=	\
	$(SRS2NAME).src

RESLIB2NAME=$(SRS2NAME)
RESLIB2SRSFILES= \
	$(SRS)$/$(SRS2NAME).srs

# Version "For internal use only"
SRS3NAME= for
SRC3FILES=	\
	$(SRS3NAME).src

RESLIB3NAME=$(SRS3NAME)
RESLIB3SRSFILES= \
	$(SRS)$/$(SRS3NAME).srs

# Version "OpenOffice"
SRS4NAME= ooo
SRC4FILES=	\
	$(SRS4NAME).src

RESLIB4NAME=$(SRS4NAME)
RESLIB4SRSFILES= \
	$(SRS)$/$(SRS4NAME).srs

.IF "$(GUIBASE)"=="java"
RSCDEFS += -DPRODUCT_NAME="$(PRODUCT_NAME)" -DPRODUCT_VERSION="$(PRODUCT_VERSION)"
SRS5NAME= neojava
SRC5FILES=	\
	$(SRS5NAME).src
.ENDIF

RESLIB5NAME=$(SRS5NAME)
RESLIB5SRSFILES= \
	$(SRS)$/$(SRS5NAME).srs

.INCLUDE :  target.mk

ALLTAR : $(ADD_IMGLSTTARGET)

