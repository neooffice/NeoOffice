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
# Modified September 2013 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..
PRJNAME=unotools
TARGET=utl
TARGETTYPE=CUI
USE_LDUMP2=TRUE

USE_DEFFILE=TRUE

# --- Settings ----------------------------------

.INCLUDE :	settings.mk

# --- Library -----------------------------------

LIB1TARGET=$(SLB)$/untools.lib
LIB1FILES=\
		$(SLB)$/i18n.lib \
		$(SLB)$/misc.lib \
		$(SLB)$/streaming.lib \
		$(SLB)$/config.lib \
        $(SLB)$/ucbhelp.lib \
		$(SLB)$/procfact.lib \
		$(SLB)$/property.lib \
		$(SLB)$/accessibility.lib

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1IMPLIB=iutl
SHL1USE_EXPORTS=name

SHL1STDLIBS= \
		$(SALHELPERLIB) \
		$(COMPHELPERLIB) \
		$(UCBHELPERLIB) \
		$(CPPUHELPERLIB) \
		$(CPPULIB) \
		$(I18NISOLANGLIB) \
		$(TOOLSLIB) \
		$(VOSLIB) \
		$(SALLIB)

.IF "$(GUIBASE)" == "java"
SHL1STDLIBS += \
		-framework CoreFoundation
.ENDIF		# "$(GUIBASE)" == "java"

SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1DEPN=$(LIB1TARGET)

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN        =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME     =untools
DEF1DES         =unotools

# --- Targets ----------------------------------

.INCLUDE : target.mk

# --- Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
	@echo ------------------------------
	@echo Making: $@
	@echo CLEAR_THE_FILE		> $@
	@echo _TI					>> $@
	@echo _real					>> $@
	@echo NodeValueAccessor			>> $@
	@echo SubNodeAccess				>> $@
	@echo UpdateFromConfig			>> $@
	@echo UpdateToConfig				>> $@
        @echo _Impl >> $@

