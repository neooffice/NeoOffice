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
# Modified August 2015 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..$/..
PRJNAME=shell
TARGET=exec

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

COMP1TYPELIST=syssh

TESTAPP1=urltest

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

DLLPRE=

SLOFILES=$(SLO)$/shellexec.obj\
    $(SLO)$/shellexecentry.obj
    
.IF "$(GUIBASE)" == "java"
SLOFILES += \
	$(SLO)$/shellexec_cocoa.obj
.ENDIF		# "$(GUIBASE)" == "java"

SHL1OBJS=$(SLOFILES) 
	
SHL1TARGET=syssh.uno
.IF "$(GUI)" == "OS2"
SHL1IMPLIB=i$(TARGET)
.ELSE
SHL1IMPLIB=
.ENDIF

SHL1VERSIONMAP=exports.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
					
SHL1STDLIBS=$(CPPULIB)\
			$(CPPUHELPERLIB)\
			$(SALLIB)
SHL1LIBS=
SHL1DEPN=

.IF "$(GUIBASE)" == "java"
SHL1STDLIBS+=-framework AppKit
.ENDIF		# "$(GUIBASE)" == "java"

.IF "$(test)" != "" 

APP1TARGET=$(TESTAPP1)
APP1STDLIBS= $(SHL1STDLIBS)
APP1OBJS= \
	$(SLO)$/shellexec.obj \
	$(SLO)$/$(APP1TARGET).obj

.ENDIF # "$(test)" != "" 


# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

run_test : $(BIN)$/$(TESTAPP1).sh
    dmake test=t
	$(BIN)$/$(TESTAPP1) urltest.txt

$(BIN)$/$(TESTAPP1).sh : $$(@:f)
	$(COPY) $< $@
	-chmod +x $@ 
