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

PRJ=..

PRJNAME=setup2
TARGET=setup
TARGETTYPE=GUI

VERSION=$(UPD)
GEN_HID=TRUE
APP2NOSAL=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

VERINFONAME=verinfo

RSCLOCINC!:=$(PRJ)$/res -I$(RSCLOCINC)

RESLIB1NAME=set
RESLIB1SRSFILES=\
	$(SRS)$/service.srs	\
	$(SRS)$/uibase.srs	\
	$(SRS)$/pages.srs	\
	$(SRS)$/ui.srs		\
	$(SRS)$/dialog.srs

RESLIB2NAME=set_pp1
RESLIB2SRSFILES=\
	$(SRS)$/ui.srs		\
	$(SRS)$/productpatch1.srs

# --- setup dynamic link library -----------------------------------------

LIB1TARGET = $(SLB)$/set.lib
LIB1FILES = \
		$(SLB)$/pages.lib      	\
		$(SLB)$/agenda.lib     	\
		$(SLB)$/compiler.lib	\
		$(SLB)$/sibasic.lib    	\
		$(SLB)$/other.lib      	\
		$(SLB)$/sicustom.lib	\
		$(SLB)$/service.lib	\
		$(SLB)$/uibase.lib

.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
.IF "$(GUIBASE)"=="aqua"
LIB1FILES += $(SLB)$/aquaos.lib
.ENDIF
.IF "$(GUIBASE)"=="unx" || "$(GUIBASE)"=="java"
LIB1FILES += $(SLB)$/mowos.lib
.ENDIF
.ELSE
LIB1FILES += $(SLB)$/mowos.lib
.ENDIF
.ENDIF
.IF "$(GUIBASE)"=="WIN"
LIB1FILES += $(SLB)$/winos.lib
.ENDIF

# ---------------------------------

SHL1TARGET = set$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB = iset
SHL1LIBS = $(LIB1TARGET)
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

SHL1DEPN = $(L)$/itools.lib $(SVLIBDEPEND)

SHL1STDLIBS= \
		$(VOSLIB) \
		$(SALLIB) \
		$(SOTLIB) \
		$(TOOLSLIB) \
		$(VCLLIB) \
		$(SVTOOLLIB) \
		$(SVLLIB) \
		$(BASICLIGHTLIB) \
		$(CPPUHELPERLIB) \
		$(COMPHELPERLIB) \
		$(CPPULIB) \
		$(SALHELPERLIB)

.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="MACOSX"
.IF "$(GUIBASE)"=="unx" || "$(GUIBASE)"=="java"
SHL1STDLIBS += \
	-Bdynamic $(THREADLIB) \
	-lX11 \
	-lXext
.ENDIF
.IF "$(GUIBASE)"=="aqua"
SHL1STDLIBS += \
	-Bdynamic $(THREADLIB) \
	-framework Cocoa
.ENDIF			# "$(GUIBASE)"=="aqua"
.ELSE
SHL1STDLIBS += \
	-Bdynamic $(THREADLIB) \
	-lX11 \
	-lXext
.ENDIF
.ELSE			# "$(GUI)"=="UNX"
SHL1OBJS = $(SLO)$/agenda.obj

SHL1STDLIBS+= \
		$(SVUNZIPDLL)
.ENDIF			# "$(GUI)"=="UNX"

.IF "$(GUI)"=="WNT"
SHL1STDLIBS += \
		shell9x.lib tools32.lib advapi32.lib gdi32.lib shell32.lib ole32.lib uuid.lib version.lib \
		winmm.lib advapi9x.lib
.ENDIF			# "$(GUI)"=="UNX"

DEF1NAME    =$(SHL1TARGET)
DEF1DEPN    =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =set
DEF1DES     =S.E.T.U.P.

DEF1EXPORT1 = component_writeInfo
DEF1EXPORT2 = component_getFactory
DEF1EXPORT3 = component_getImplementationEnvironment

# --- setup binary --------------------------------------------------------

APP2TARGET = $(TARGET)


.IF "$(GUI)"=="WNT"
APP2ICON=$(SOLARRESDIR)$/icons$/500_setup.ico
APP2VERINFO=verinfo.rc
APP2LINKRES=$(MISC)$/$(TARGET).res
.ENDIF

APP2LIBS+=\
		$(LB)$/ui.lib			\
		$(LB)$/dialog.lib		\
		$(LB)$/iset.lib

APP2STDLIBS=\
		$(COMPHELPERLIB) $(CPPUHELPERLIB) $(CPPULIB) $(VOSLIB) $(SALLIB) $(SOTLIB) $(UNOLIB) \
		$(TOOLSLIB) $(SVLIB) $(SVTOOLLIB) $(SVLLIB) $(BASICLIGHTLIB)

.IF "$(SOLAR_JAVA)" != ""
APP2STDLIBS+=$(SJLIB)
.ENDIF
.IF "$(GUI)"=="WNT"
APP2STDLIBS += advapi32.lib gdi32.lib shell32.lib ole32.lib uuid.lib version.lib \
		winmm.lib advapi9x.lib
.ENDIF

.IF "$(GUI)"=="UNX"

APP2DEPN+= $(LB)$/libset$(UPD)$(DLLPOSTFIX)$(DLLPOST)

.IF "$(OS)"=="LINUX" ||"$(OS)"=="FREEBSD" || "$(OS)"=="NETBSD" || "$(OS)"=="FREEBSD"
APP2STDLIBS+= 	\
	$(RTLLIB)	\
	$(TECLIB)	\
	$(TKLIB)	\
	$(CPPULIB)
.ENDIF

.IF "$(OS)"=="MACOSX"
APP2STDLIBS+= 	\
	$(RTLLIB)	\
	$(TECLIB)	\
	$(TKLIB)	\
	$(CPPULIB)	\
	$(SALHELPERLIB)
.ENDIF

.IF "$(OS)" == "SOLARIS"
APP2STDLIBS += \
	-lset$(UPD)$(DLLPOSTFIX) \
	-Bdynamic $(THREADLIB) \
	-lX11 \
	-lXext
.ENDIF

.IF "$(OS)"=="LINUX" ||"$(OS)"=="FREEBSD" || "$(OS)"=="NETBSD" || "$(OS)"=="IRIX" || "$(OS)"=="FREEBSD"
APP2STDLIBS+= 	\
	-lset$(UPD)$(DLLPOSTFIX) \
	-Wl,-Bdynamic $(THREADLIB) \
	-lX11 \
	-lXext \
	-lSM \
	-lICE
.ENDIF

.IF "$(OS)"=="MACOSX"
.IF "$(GUIBASE)"=="unx" || "$(GUIBASE)"=="java"
APP2STDLIBS+=  \
	-lset$(UPD)$(DLLPOSTFIX) \
	-lXt \
	-lX11 \
	-lXext \
	-lXmu \
	-lSM \
	-lICE
.ENDIF
.IF "$(GUIBASE)"=="aqua"
APP2STDLIBS+=  \
	-lset$(UPD)$(DLLPOSTFIX)
.ENDIF
.ENDIF

.ENDIF

APP2DEPN+=\
	$(LB)$/ui.lib 				\
	$(SLB)$/pages.lib			\
	$(LB)$/dialog.lib			\
	verinfo.rc					\
	$(APP1RES)					\
	makefile.mk

.IF "$(GUI)" != "UNX"
APP2OBJS+=\
		$(OBJ)$/main.obj		\
		$(OBJ)$/mainwnd.obj		\
		$(OBJ)$/mainevt.obj		\
		$(OBJ)$/magenda.obj		\
		$(OBJ)$/textani.obj
.ENDIF

APP2DEF=   $(MISCX)$/$(TARGET).def

# --- setup --------------------------------------------------------------


.IF "$(GUI)"!="UNX"

#
#	Loader
#

APP3NOSAL=TRUE
APP3TARGET=loader
.IF "$(GUI)"=="WNT"
APP3STDLIBS=svunzip.lib gdi32.lib advapi32.lib libcmt.lib shell32.lib \
	$(SALLIB)

.ENDIF
.IF "$(GUI)"=="WNT"
APP3OBJS = $(OBJ)$/loader.obj $(OBJ)$/larch.obj
.ELSE
APP3OBJS = $(OBJ)$/loader.obj $(OBJ)$/arch.obj
.ENDIF
APP3DEF = $(MISCX)$/loader.def
APP3RES = $(RES)$/loader.res
APP3NOSVRES = $(RES)$/loader.res
APP3DEPN = $(APP3TARGETN:d:+"unloader.exe")

.ENDIF			# "$(GUI)"!="UNX"

.INCLUDE :  target.mk

# -------------------------------------------------------------------------
# --- Targets -------------------------------------------------------------

ALLTAR : $(BIN)$/setup_services.rdb

UPDATE_TEST : makefile.mk setup.inf f_zip f_zipdir
	@echo ------------------------------
.IF "$(GUI)"!="MAC"
.IF "$(UPDATER)"=="YES"
	+-$(COPY) setup.inf $(BIN)$
	+-$(COPY) setup.inf $(BIN)$/setup.ins
	+-$(COPY) f_zip $(BIN)$
	+-$(COPY) f_zipdir $(BIN)$
	+-$(COPY) $(SOLARBINDIR)$/vcl???49.res $(BIN)$
.ENDIF
.ENDIF

# -------------------------------------------------------------------------
# Filter fuer set?????.dll
# -------------------------------------------------------------------------
$(MISC)$/$(SHL1TARGET).flt : makefile.mk
    @echo ------------------------------
    @echo Making: 						$@
    @echo Impl							>$@
    @echo Registry_Api					>>$@


# -------------------------------------------------------------------------
# If you add something here, please also add it to sip/util/makefile.mk!
# -------------------------------------------------------------------------

REGISTERLIBS= \
	$(DLLPRE)i18n$(UPD)$(DLLPOSTFIX)$(DLLPOST) \
	$(DLLPRE)i18npool$(UPD)$(DLLPOSTFIX)$(DLLPOST) \
	$(DLLPRE)mcnttype$(DLLPOST)

.IF "$(GUI)" == "UNX"
.IF "$(OS)"=="MACOSX"
.IF "$(GUIBASE)"=="aqua"
REGISTERLIBS+= \
	$(DLLPRE)dtransaqua$(UPD)$(DLLPOSTFIX)$(DLLPOST)
.ELSE
REGISTERLIBS+= \
	$(DLLPRE)dtransX11$(UPD)$(DLLPOSTFIX)$(DLLPOST)
.ENDIF # GUIBASE == aqua
.ELSE
REGISTERLIBS+= \
	$(DLLPRE)dtransX11$(UPD)$(DLLPOSTFIX)$(DLLPOST)
.ENDIF # OS == MACOSX

$(BIN)$/setup_services.rdb: \
	$(SOLARLIBDIR)$/$(DLLPRE)i18n$(UPD)$(DLLPOSTFIX)$(DLLPOST) \
	$(SOLARLIBDIR)$/$(DLLPRE)i18npool$(UPD)$(DLLPOSTFIX)$(DLLPOST) \
	$(SOLARLIBDIR)$/$(DLLPRE)mcnttype$(DLLPOST) \
	$(SOLARLIBDIR)$/$(DLLPRE)dtransX11$(UPD)$(DLLPOSTFIX)$(DLLPOST)

$(BIN)$/setup_services.rdb: makefile.mk
	regcomp -register -r $@ -c "$(strip $(REGISTERLIBS))"

.ELSE			# "$(GUI)" == "UNX"
.IF "$(GUI)"=="WNT"

REGISTERLIBS+= \
	$(DLLPRE)sysdtrans$(DLLPOST) \
	$(DLLPRE)ftransl$(DLLPOST) \
	$(DLLPRE)dnd$(DLLPOST)

$(BIN)$/setup_services.rdb: \
	$(SOLARBINDIR)$/$(DLLPRE)i18n$(UPD)$(DLLPOSTFIX)$(DLLPOST) \
	$(SOLARBINDIR)$/$(DLLPRE)i18npool$(UPD)$(DLLPOSTFIX)$(DLLPOST) \
	$(SOLARBINDIR)$/$(DLLPRE)mcnttype$(DLLPOST) \
	$(SOLARBINDIR)$/$(DLLPRE)sysdtrans$(DLLPOST) \
	$(SOLARBINDIR)$/$(DLLPRE)ftransl$(DLLPOST) \
	$(SOLARBINDIR)$/$(DLLPRE)dnd$(DLLPOST)

$(BIN)$/setup_services.rdb: makefile.mk
	regcomp -register -r $@ -c "$(strip $(REGISTERLIBS))"

.ELSE			# "$(GUI)"=="WNT"

	@echo "**********************************************************"
	@echo "*** unknown platform: don't know which librarys to use ***"
	@echo "**********************************************************"
	force_dmake_to_error
.ENDIF			# "$(GUI)"=="WNT"
.ENDIF			# "$(GUI)" == "UNX"

