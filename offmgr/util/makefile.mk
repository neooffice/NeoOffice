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
#   Modified January 2005 by Edward Peterlin. SISSL Removed. NeoOffice is
#   distributed under GPL only under modification term 3 of the LGPL.
# 
#   Contributor(s): _______________________________________
# 
##########################################################################

PRJ=..

PRJNAME=ofa
TARGET=ofa
GEN_HID=TRUE
GEN_HID_OTHER=TRUE
.IF "$(CPU)"=="i386"
USE_LDUMP2=TRUE
.ENDIF

PROJECTPCH=ofapch
PDBTARGET=ofapch
PROJECTPCHSOURCE=$(PRJ)$/util$/ofapch

# --- Settings ------------------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(prjpch)" != ""
CDEFS+=-DPRECOMPILED
.ENDIF

RSCLOCINC!:=$(RSCLOCINC);$(PRJ)$/RES
SOLARLIB+=-L$(LB)

# --- Allgemein -----------------------------------------------------------

RES1FILELIST=\
	$(SRS)$/ofaslots.srs \
	$(SRS)$/dialog.srs \
	$(SRS)$/app.srs \
	$(SOLARRESDIR)$/sfxslots.srs \
	$(SOLARRESDIR)$/svxslots.srs \
	$(SOLARRESDIR)$/svtools.srs \
	$(SOLARRESDIR)$/sfx.srs 	\
	$(SOLARRESDIR)$/so2.srs 	\
	$(SOLARRESDIR)$/basic.srs

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RES1FILELIST)

# -------------------------------------------------------------------------

LIB3TARGET= $(SLB)$/$(TARGET).lib
LIB3FILES=	$(SLB)$/app.lib 		\
			$(SLB)$/dialog.lib

#.IF "$(GUI)"!="UNX"
#LIB3FILES+= \
#	$(LIBPRE) $(SOLARLIBDIR)$/ysch.lib	\
#	$(LIBPRE) $(SOLARLIBDIR)$/ysm.lib
#.ENDIF # "$(GUI)"!="UNX"

SHL2TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
SHL2IMPLIB= _$(TARGET)
SHL2LIBS=	$(SLB)$/$(TARGET).lib

.IF "$(OS)"=="MACOSX"
# Static libs must go at end of library list
SHL2STDLIBS=	$(BASICLIB) \
			$(SVXLIB)   \
			$(SFX2LIB)
.ELSE
# static libraries
SHL2STDLIBS=   $(BASICIDELIB) \
			$(SVXLIB)		\
			$(SFX2LIB) \
			$(BASICLIB)
.ENDIF

#.IF "$(GUI)"=="UNX"
#SHL2STDLIBS+= \
#	$(SCHLIB) \
#	$(SMLIB)
#.ENDIF # "$(GUI)" == "UNX"

# dynamic libraries
SHL2STDLIBS+= \
	$(GOODIESLIB) \
	$(SVTOOLLIB) \
	$(SVLLIB)	\
	$(VCLLIB) \
	$(TOOLSLIB) \
	$(UNOTOOLSLIB) \
	$(COMPHELPERLIB) \
	$(CPPUHELPERLIB) \
	$(CPPULIB) \
	$(VOSLIB) \
	$(SALLIB)

.IF "$(OS)"=="MACOSX"
SHL2STDLIBS+= $(BASICIDELIB)
.ENDIF

# [ed] 1/25/05 Add Carbon for prefs support.  Bug 396
.IF "$(GUIBASE)"=="java"
.IF "$(OS)"=="MACOSX"
SHL2STDLIBS += -framework ApplicationServices -framework Carbon
.ENDIF
.ENDIF

.IF "$(GUI)"=="WNT"
SHL2STDLIBS += $(LIBPRE) advapi32.lib
.ENDIF # WNT

SHL2DEF=	$(MISC)$/$(SHL2TARGET).def
SHL2BASE=	0x1de00000

DEF2NAME=	$(SHL2TARGET)
DEF2DEPN=	$(MISC)$/$(SHL2TARGET).flt
DEFLIB2NAME =$(TARGET)
DEF2DES 	=offmgr app-interface

# -------------------------------------------------------------------------

.IF "$(GUI)"=="WNT"
LIB4TARGET= $(LB)$/a$(TARGET).lib
LIB4FILES=	$(LB)$/_$(TARGET).lib
.ENDIF


.IF "$(BUILD_SOSL)"==""
.IF "$(depend)" == ""
ALL:\
	$(INCCOM)$/sba.hrc		\
	$(LIB3TARGET)			\
	$(INCCOM)$/class.lst	\
	ALLTAR

.ENDIF # "$(depend)" == ""
.ENDIF # "$(BUILD_SOSL)" == ""

.INCLUDE :	target.mk

$(MISCX)$/$(SHL2TARGET).flt: makefile.mk
	@echo ------------------------------
	@echo Making: $@
	@echo WEP>$@
	@echo PlugInApplication>>$@
	@echo __dt__17OfficeApplication>>$@
	@echo LibMain>>$@
	@echo _Impl>>$@
	@echo _Imp>>$@
	@echo solver>>$@
	@echo bad_alloc>>$@
	@echo exception12>>$@
.IF "$(GUI)"=="WNT"
	@echo __CT>>$@
.ENDIF

# ------------------------------------------------------------------
# Windows NT
# ------------------------------------------------------------------

$(INCCOM)$/class.lst:
.IF "$(BUILD_SOSL)"==""
.IF "$(GUI)$(CPU)$(UPDATER)"=="WNTIYES"
	+-$(COPY) class.lst $@
.ELSE
	@+echo nix
.ENDIF
.ENDIF			# "$(BUILD_SOSL)"==""

$(INCCOM)$/sba.hrc: $(INC)$/sbasltid.hrc
	@+-$(COPY) $(INC)$/sbasltid.hrc $(INCCOM)$/sba.hrc


