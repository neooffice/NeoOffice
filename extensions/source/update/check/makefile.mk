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
# Modified January 2009 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************
PRJ=..$/..$/..

PRJNAME=extensions
TARGET=updchk
TARGETTYPE=CUI
PACKAGE=org.openoffice.Office

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

ABSXCSROOT=$(SOLARXMLDIR)
XCSROOT=..
DTDDIR=$(ABSXCSROOT)
XSLDIR=$(ABSXCSROOT)$/processing
PROCESSOUT=$(MISC)$/$(TARGET)
PROCESSORDIR=$(SOLARBINDIR)

# no validation by inspector class
NO_INSPECTION=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# no "lib" prefix
DLLPRE =

# --- Files ---

.IF "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"
APP1TARGET=$(TARGET)runinstallers
APP1OBJS=$(OBJ)$/updateruninstallers.obj
.ENDIF		# "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"

SRS2NAME=$(TARGET)
SRC2FILES=\
	updatehdl.src

RESLIB2NAME=upd
RESLIB2SRSFILES= $(SRS)$/updchk.srs
RESLIB2DEPN= updatehdl.src updatehdl.hrc

SLOFILES=\
	$(SLO)$/download.obj \
	$(SLO)$/updatecheck.obj \
	$(SLO)$/updatecheckjob.obj \
	$(SLO)$/updatecheckconfig.obj \
	$(SLO)$/updateprotocol.obj \
	$(SLO)$/updatehdl.obj
        
.IF "$(GUIBASE)" == "java"
SLOFILES += \
	$(SLO)$/update_cocoa.obj \
	$(SLO)$/updatei18n_cocoa.obj \
	$(SLO)$/updatewebview_cocoa.obj
.ENDIF		# "$(GUIBASE)" == "java"

SHL1NOCHECK=TRUE
SHL1TARGET=$(TARGET).uno   
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1IMPLIB=i$(SHL1TARGET)
SHL1STDLIBS=    \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(CURLLIB) \
        $(SALLIB) \
        $(SHELL32LIB) \
        $(OLE32LIB)

.IF "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"
SHL1STDLIBS += \
	$(UNOTOOLSLIB) \
	$(VCLLIB)
.IF "$(GUIBASE)" == "java"
SHL1STDLIBS += \
	$(SFXLIB) \
	$(TOOLSLIB)
SHL1STDLIBS += -framework Cocoa -framework WebKit
.ENDIF		# "$(GUIBASE)" == "java"
.ENDIF		# "$(GUIBASE)" == "java" || "$(GUIBASE)" == "WIN"
        
SHL1VERSIONMAP=..$/exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

PACKAGEDIR=$(subst,.,$/ $(PACKAGE))
SPOOLDIR=$(MISC)$/registry$/spool

XCUFILES= \
	Addons.xcu \
	Jobs.xcu

MYXCUFILES= \
	$(SPOOLDIR)$/$(PACKAGEDIR)$/Addons$/Addons-onlineupdate.xcu \
	$(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-onlineupdate.xcu

LOCALIZEDFILES= \
	Addons.xcu \
	Jobs.xcu

.IF "$(test)" != ""
APP1TARGET=updateprotocoltest
APP1STDLIBS= $(SHL1STDLIBS)
APP1OBJS= \
	$(SLO)$/updateprotocol.obj \
	$(SLO)$/updateprotocoltest.obj


.ENDIF # "$(test)" != ""

# --- Targets ---

.INCLUDE : target.mk

ALLTAR : $(MYXCUFILES)

.IF "$(WITH_LANG)"!=""
XCU_SOURCEDIR:=$(PROCESSOUT)$/merge$/$(PACKAGEDIR)
.ELSE			# "$(WITH_LANG)"!=""
XCU_SOURCEDIR:=.
.ENDIF			# "$(WITH_LANG)"!=""

$(SPOOLDIR)$/$(PACKAGEDIR)$/Addons$/Addons-onlineupdate.xcu : $(XCU_SOURCEDIR)$/Addons.xcu
	@-$(MKDIRHIER) $(@:d)
	@$(COPY) $< $@

$(SPOOLDIR)$/$(PACKAGEDIR)$/Jobs$/Jobs-onlineupdate.xcu : $(XCU_SOURCEDIR)$/Jobs.xcu
	@-$(MKDIRHIER) $(@:d)
	@$(COPY) $< $@
#	@$(PERL) transform.pl < $< > $@

