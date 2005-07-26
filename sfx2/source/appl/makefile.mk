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
#   Modified July 2005 by Patrick Luby. SISSL Removed. NeoOffice is
#   distributed under GPL only under modification term 3 of the LGPL.
# 
#   Contributor(s): _______________________________________
# 
##########################################################################

PRJ=..$/..

PRJNAME=sfx2
TARGET=appl
ENABLE_EXCEPTIONS=TRUE
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

IMGLST_SRS=$(SRS)$/appl.srs
BMP_IN=$(PRJ)$/win/res

# w.g. compilerbugs
.IF "$(GUI)"=="WNT"
CFLAGS+=-Od
.ENDIF

# --- Files --------------------------------------------------------

SRS1NAME=appl
SRC1FILES =  \
		app.src sfx.src image.src newhelp.src

SRS2NAME=sfx
SRC2FILES =  \
		sfx.src

SLOFILES =  \
	$(SLO)$/imagemgr.obj\
    $(SLO)$/appopen.obj \
	$(SLO)$/appuno.obj \
	$(SLO)$/appmail.obj \
	$(SLO)$/appmain.obj \
	$(SLO)$/appinit.obj \
	$(SLO)$/appmisc.obj \
	$(SLO)$/appdemo.obj \
	$(SLO)$/appreg.obj \
	$(SLO)$/appcfg.obj \
	$(SLO)$/appquit.obj \
	$(SLO)$/appchild.obj \
	$(SLO)$/appserv.obj \
	$(SLO)$/appdata.obj \
	$(SLO)$/app.obj \
	$(SLO)$/appbas.obj \
	$(SLO)$/appdde.obj \
	$(SLO)$/workwin.obj \
	$(SLO)$/sfxhelp.obj \
	$(SLO)$/childwin.obj \
	$(SLO)$/sfxdll.obj \
	$(SLO)$/module.obj \
	$(SLO)$/appsys.obj \
	$(SLO)$/loadenv.obj \
	$(SLO)$/dlgcont.obj \
	$(SLO)$/namecont.obj \
	$(SLO)$/scriptcont.obj \
	$(SLO)$/newhelp.obj \
	$(SLO)$/helpinterceptor.obj \
	$(SLO)$/shutdownicon.obj \
	$(SLO)$/shutdowniconw32.obj \
	$(SLO)$/sfxpicklist.obj \
	$(SLO)$/helpdispatch.obj \
    $(SLO)$/imestatuswindow.obj \
    $(SLO)$/accelinfo.obj

EXCEPTIONSFILES=\
	$(SLO)$/imagemgr.obj		\
	$(SLO)$/appopen.obj \
	$(SLO)$/appmain.obj			\
	$(SLO)$/appmisc.obj			\
	$(SLO)$/frstinit.obj		\
	$(SLO)$/appinit.obj			\
	$(SLO)$/appcfg.obj			\
	$(SLO)$/helpinterceptor.obj	\
	$(SLO)$/newhelp.obj			\
	$(SLO)$/sfxhelp.obj			\
	$(SLO)$/shutdownicon.obj	\
	$(SLO)$/shutdowniconw32.obj \
	$(SLO)$/sfxpicklist.obj		\
	$(SLO)$/helpdispatch.obj

.IF "$(GUI)" == "MAC"
SLOFILES +=\
		$(SLO)$/appmac.obj
.ENDIF

.IF "$(GUIBASE)" == "java"
.IF "$(OS)" == "MACOSX"
SLOFILES +=  \
	$(SLO)$/shutdownicon_cocoa.obj
.ENDIF
.ENDIF
# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

