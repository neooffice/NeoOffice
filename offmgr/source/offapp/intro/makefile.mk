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
#   Modified June 2003 by Patrick Luby. SISSL Removed. NeoOffice is
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

.INCLUDE :  $(UPD)minor.mk
RSCUPDVER=$(RSCREVISION)

#RSCUPDVER=$(_SDT_REVISION)(SV$(UPD)$(UPDMINOR))
IMGLST_SRS=$(SRS)$/iso.srs
ADD_IMGLST_SRS=$(SRS)$/ooo.srs $(SRS)$/oem.srs

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
# Version "NeoOffice/J"
RSCDEFS += -DPRODUCT_NAME="$(PRODUCT_NAME)" -DPRODUCT_VERSION="$(PRODUCT_VERSION)"
SRS5NAME= neojava
SRC5FILES=	\
	$(SRS5NAME).src
.ELSE
# Version "NeoOffice"
SRS5NAME= neo
SRC5FILES=	\
	$(SRS5NAME).src
.ENDIF

RESLIB5NAME=$(SRS5NAME)
RESLIB5SRSFILES= \
	$(SRS)$/$(SRS5NAME).srs

.IF "$(ADD_IMGLST_SRS)"!=""
.IF "$(make_srs_deps)"==""
.IF "$(common_build_reslib)"!=""
ADD_IMGLSTTARGET=$(foreach,j,$(ADD_IMGLST_SRS) $(foreach,i,$(alllangext) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(MISC))$/$(j:b)_img$i.don))
.ELSE			# "$(common_build_reslib)"!=""
ADD_IMGLSTTARGET=$(foreach,j,$(ADD_IMGLST_SRS) $(foreach,i,$(alllangext) $(MISC)$/$(j:b)_img$i.don))
.ENDIF			# "$(common_build_reslib)"!=""
.ENDIF			# "$(make_srs_deps)"==""
.ENDIF			# "$(ADD_IMGLST_SRS)"!=""


.INCLUDE :  target.mk

ALLTAR : $(ADD_IMGLSTTARGET)

.IF "$(RESLIB4TARGETN)"!=""
$(RESLIB4TARGETN) :  $(ADD_IMGLSTTARGET)
.ENDIF			# "$(RESLIB4TARGETN)"!=""

.IF "$(ADD_IMGLSTTARGET)"!=""
$(ADD_IMGLSTTARGET): $(ADD_IMGLST_SRS)
	@+echo -----------------
	@+echo Making Imagelists:
	@+echo -----------------
	@+-$(RM) $@ >& $(NULLDEV)
.IF "$(common_build_reslib)"!=""
	@-+$(MKDIR) $(RES)$/$(langext_{$(subst,$(@:b:s/_img/ /:1)_img, $(@:b))}) >& $(NULLDEV)
	@-+$(MKDIR) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))$/$(langext_{$(subst,$(@:b:s/_img/ /:1)_img, $(@:b))}) >& $(NULLDEV)
	+$(BMP) $(SRS)$/$(@:b:s/_img/ /:1).srs $(BMP_IN) $(BMP_OUT)$/$(langext_{$(subst,$(@:b:s/_img/ /:1)_img, $(@:b))}) $(lang_{$(subst,$(@:b:s/_img/ /:1)_img, $(@:b))}) -f $@
	-+$(GNUCOPY) -pub $(RES)$/$(langext_{$(subst,$(@:b:s/_img/ /:1)_img, $(@:b))})/* $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))$/$(langext_{$(subst,$(@:b:s/_img/ /:1)_img, $(@:b))}) >& $(NULLDEV)
	+-$(RM) $(subst,$(OUTPATH),$(COMMON_OUTDIR) $(RES))$/$(langext_{$(subst,$(@:b:s/_img/ /:1)_img, $(@:b))})$/*.bmp~
.ELSE			# "$(common_build_reslib)"!=""
	@-+$(MKDIR) $(RES)$/$(langext_{$(subst,$(@:b:s/_img/ /:1)_img, $(@:b))}) >& $(NULLDEV)
	+$(BMP) $(SRS)$/$(@:b:s/_img/ /:1).srs $(BMP_IN) $(BMP_OUT)$/$(langext_{$(subst,$(@:b:s/_img/ /:1)_img, $(@:b))}) $(lang_{$(subst,$(@:b:s/_img/ /:1)_img, $(@:b))}) -f $@
.ENDIF			# "$(common_build_reslib)"!=""
.IF "$(BMP_WRITES_FLAG)"==""
	@+echo > $@
.ENDIF
.ENDIF
