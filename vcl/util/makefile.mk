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

PRJ=..

PRJNAME=vcl
TARGET=vcl
VERSION=$(UPD)
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  makefile.pmk


# --- Allgemein ----------------------------------------------------------

HXXDEPNLST= $(INC)$/accel.hxx       \
            $(INC)$/animate.hxx     \
            $(INC)$/apptypes.hxx    \
            $(INC)$/bitmap.hxx      \
            $(INC)$/bitmapex.hxx    \
            $(INC)$/bmpacc.hxx      \
            $(INC)$/btndlg.hxx      \
            $(INC)$/button.hxx      \
            $(INC)$/ctrl.hxx        \
            $(INC)$/color.hxx       \
            $(INC)$/config.hxx      \
            $(INC)$/cursor.hxx      \
            $(INC)$/cmdevt.hxx      \
            $(INC)$/decoview.hxx    \
            $(INC)$/dialog.hxx      \
            $(INC)$/dockwin.hxx     \
            $(INC)$/edit.hxx        \
            $(INC)$/event.hxx       \
            $(INC)$/field.hxx       \
            $(INC)$/fixed.hxx       \
            $(INC)$/floatwin.hxx    \
            $(INC)$/font.hxx        \
            $(INC)$/fontcvt.hxx     \
            $(INC)$/floatwin.hxx    \
            $(INC)$/graph.hxx       \
            $(INC)$/group.hxx       \
            $(INC)$/help.hxx        \
            $(INC)$/jobset.hxx      \
            $(INC)$/keycodes.hxx    \
            $(INC)$/keycod.hxx      \
            $(INC)$/image.hxx       \
            $(INC)$/line.hxx        \
            $(INC)$/lstbox.h        \
            $(INC)$/lstbox.hxx      \
            $(INC)$/mapmod.hxx      \
            $(INC)$/metaact.hxx     \
            $(INC)$/menu.hxx        \
            $(INC)$/menubtn.hxx     \
            $(INC)$/metric.hxx      \
            $(INC)$/morebtn.hxx     \
            $(INC)$/msgbox.hxx      \
            $(INC)$/octree.hxx      \
            $(INC)$/outdev.hxx      \
            $(INC)$/outdev3d.hxx    \
            $(INC)$/pointr.hxx      \
            $(INC)$/poly.hxx        \
            $(INC)$/ptrstyle.hxx    \
            $(INC)$/prntypes.hxx    \
            $(INC)$/print.hxx       \
            $(INC)$/prndlg.hxx      \
            $(INC)$/region.hxx      \
            $(INC)$/rc.hxx          \
            $(INC)$/resid.hxx       \
            $(INC)$/resary.hxx      \
            $(INC)$/salbtype.hxx    \
            $(INC)$/scrbar.hxx      \
            $(INC)$/slider.hxx      \
            $(INC)$/seleng.hxx      \
            $(INC)$/settings.hxx    \
            $(INC)$/sound.hxx       \
            $(INC)$/sndstyle.hxx    \
            $(INC)$/split.hxx       \
            $(INC)$/splitwin.hxx    \
            $(INC)$/spin.hxx        \
            $(INC)$/spinfld.hxx     \
            $(INC)$/status.hxx      \
            $(INC)$/stdtext.hxx     \
            $(INC)$/sv.h            \
            $(INC)$/svapp.hxx       \
            $(INC)$/syschild.hxx    \
            $(INC)$/sysdata.hxx     \
            $(INC)$/system.hxx      \
            $(INC)$/syswin.hxx      \
            $(INC)$/tabctrl.hxx     \
            $(INC)$/tabdlg.hxx      \
            $(INC)$/tabpage.hxx     \
            $(INC)$/toolbox.hxx     \
            $(INC)$/timer.hxx       \
            $(INC)$/virdev.hxx      \
            $(INC)$/wall.hxx        \
            $(INC)$/waitobj.hxx     \
            $(INC)$/wintypes.hxx    \
            $(INC)$/window.hxx      \
            $(INC)$/wrkwin.hxx

.IF "$(linkinc)" != ""
SHL11FILE=  $(MISC)$/app.slo
SHL12FILE=  $(MISC)$/gdi.slo
SHL13FILE=  $(MISC)$/win.slo
SHL14FILE=  $(MISC)$/ctrl.slo
#SHL15FILE=  $(MISC)$/ex.slo
SHL16FILE=  $(MISC)$/salapp.slo
SHL17FILE=  $(MISC)$/salwin.slo
SHL18FILE=  $(MISC)$/salgdi.slo
.ENDIF

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=  $(SLB)$/app.lib     \
            $(SLB)$/gdi.lib     \
            $(SLB)$/win.lib     \
            $(SLB)$/ctrl.lib    \
            $(SLB)$/helper.lib


.IF "$(remote)" != ""
    LIB1FILES+= $(SLB)$/remote.lib
.IF "$(COM)"=="GCC"
LIB1OBJFILES=$(SLO)$/salmain.obj
.ENDIF
.ELSE           # "$(remote)" != ""
LIB1FILES+= \
            $(SLB)$/salwin.lib  \
            $(SLB)$/salgdi.lib  \
            $(SLB)$/salapp.lib
.ENDIF          # "$(remote)" != ""

.IF "$(GUI)" == "UNX"
.IF "$(GUIBASE)"=="java"
    LIB1FILES += $(SLB)$/saljava.lib
.ELSE
.IF "$(USE_XPRINT)" != "TRUE"
    SHL1STDLIBS=-lpsp$(VERSION)$(DLLPOSTFIX)
.ENDIF # ! USE_XPRINT
.ENDIF # java
.ENDIF # UNX

SHL1TARGET= vcl$(VERSION)$(DLLPOSTFIX)
SHL1IMPLIB= ivcl
SHL1STDLIBS+=\
            $(SOTLIB)           \
            $(UNOTOOLSLIB)      \
            $(TOOLSLIB)         \
            $(COMPHELPERLIB)	\
            $(UCBHELPERLIB)     \
            $(CPPUHELPERLIB)    \
            $(CPPULIB)          \
            $(VOSLIB)           \
            $(SALLIB)

.IF "$(ENABLE_CTL)"!=""
    SHL1STDLIBS+= $(ICUUCLIB) $(ICULELIB)
.ENDIF # ENABLE_CTL

.IF "$(USE_BUILTIN_RASTERIZER)"!=""
    LIB1FILES +=    $(SLB)$/glyphs.lib
    SHL1STDLIBS+=   $(FREETYPELIB)
.ENDIF # USE_BUILTIN_RASTERIZER


.IF "$(GUI)"!="MAC"
SHL1DEPN=   $(L)$/itools.lib $(L)$/sot.lib
.ENDIF

SHL1LIBS=   $(LIB1TARGET)
.IF "$(GUI)"!="UNX"
SHL1OBJS=   $(SLO)$/salshl.obj
.ENDIF

.IF "$(GUI)" != "MAC"
.IF "$(GUI)" != "UNX"
SHL1RES=    $(RES)$/salsrc.res
.ENDIF
.ENDIF
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
DEF1DEPN    =   $(MISC)$/$(SHL1TARGET).flt \
                $(HXXDEPNLST) \
                $(LIB1TARGET)
DEF1DES     =VCL
DEFLIB1NAME =vcl
DEF1EXPORT1=component_getFactory
DEF1EXPORT2=component_getImplementationEnvironment
DEF1EXPORT3=component_writeInfo


# --- W32 ----------------------------------------------------------------

.IF "$(GUI)" == "WNT"

SHL1STDLIBS += uwinapi.lib      \
               gdi32.lib        \
               winspool.lib     \
               ole32.lib        \
               shell32.lib      \
               advapi32.lib     \
               apsp.lib         \
               imm32.lib

.IF "$(GUI)$(COM)$(CPU)" == "WNTMSCI"
LINKFLAGSSHL += /ENTRY:LibMain@12
.ENDIF

.ENDIF

# --- UNX ----------------------------------------------------------------

.IF "$(GUI)"=="UNX"

.IF "$(GUIBASE)"=="aqua"
SHL1STDLIBS += -framework Cocoa
.ENDIF

.IF "$(GUIBASE)"=="java"
.IF "$(OS)"=="MACOSX"
SHL1STDLIBS += -framework ApplicationServices -framework Carbon -framework AudioToolbox -framework AudioUnit -lobjc
.ENDIF
.ENDIF

.IF "$(GUIBASE)"=="unx"

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS += -ldl
.ENDIF

.IF "$(WITH_LIBSN)"=="YES"
SHL1STDLIBS+=$(LIBSN_LIBS)
.ENDIF

# Solaris
.IF "$(OS)"=="SOLARIS"

.IF "$(USE_XPRINT)" == "TRUE"
SHL1STDLIBS += -lXp -lXext -lSM -lICE -lX11
.ELSE
SHL1STDLIBS += -lXext -lSM -lICE -lX11
.ENDIF          # "$(USE_XPRINT)" == "TRUE"

# Others
.ELSE           # "$(OS)"=="SOLARIS"
.IF "$(USE_XPRINT)" == "TRUE"
SHL1STDLIBS += -lXp -lXext -lSM -lICE -lX11
.ELSE
.IF "$(CPU)" == "I"
SHL1STDLIBS += -Wl,-Bstatic -lXinerama -Wl,-Bdynamic 
.ENDIF
SHL1STDLIBS += -lXext -lSM -lICE -lX11
.ENDIF          # "$(USE_XPRINT)" == "TRUE"
.ENDIF          # "$(OS)"=="SOLARIS"
.ENDIF          # "$(GUIBASE)"=="unx"

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS += -lXinerama
.ENDIF

.IF "$(OS)"=="LINUX" || "$(OS)"=="SOLARIS" || "$(OS)"=="FREEBSD"
SHL1STDLIBS += -laudio
.IF "$(OS)"=="SOLARIS"
# needed by libaudio.a
SHL1STDLIBS += -ldl -lnsl -lsocket
.ENDIF # SOLARIS
.ENDIF          # "$(OS)"=="LINUX" || "$(OS)"=="SOLARIS" || "$(OS)"=="FREEBSD"

.ENDIF          # "$(GUI)"=="UNX"

.IF "$(GUIBASE)"=="java"
JARCLASSDIRS = com
JARTARGET = $(TARGET).jar
JARCOMPRESS = TRUE
.ENDIF

# --- Allgemein ----------------------------------------------------------

.INCLUDE :  target.mk

# --- Targets ------------------------------------------------------------

# --- VCL-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo Impl > $@
    @echo Sal>> $@
    @echo Dbg>> $@
    @echo HelpTextWindow>> $@
    @echo MenuBarWindow>> $@
    @echo MenuFloatingWindow>> $@
    @echo MenuItemList>> $@
    @echo LibMain>> $@
    @echo LIBMAIN>> $@
    @echo Wep>> $@
    @echo WEP>> $@
    @echo RmEvent>> $@
    @echo RmFrameWindow>> $@
    @echo RmPrinter>> $@
    @echo RmBitmap>> $@
    @echo RmSound>> $@
    @echo __CT>> $@
    @echo _TI2>> $@
    @echo _TI3>> $@
    @echo _real@ >> $@
    @echo xMonitorFrom >> $@
    @echo xEnumDisplay >> $@
    @echo xGetSystemMetrics >> $@
    @echo xGetMonitorInfo >> $@
    @echo WIN_ >> $@
    @echo component_ >> $@
    @echo DNDEventDispatcher>> $@
    @echo DNDListenerContainer>> $@
    @echo vcl\ >> $@

