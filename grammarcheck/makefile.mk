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
#   GNU General Public License Version 2.1
#   =============================================
#   Copyright 2007 by Planamesa Inc. -  http://www.planamesa.com
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
##########################################################################

PRJ=.

PRJNAME=grammarcheck
TARGET=grammarcheck.uno
TARGETTYPE=CUI

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES= \
	$(SLO)$/grammarcheck.obj

SHL1TARGET=$(TARGET)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS= \
	$(SALLIB) \
	$(CPPULIB) \
	$(CPPUHELPERLIB) \
	$(COMPHELPERLIB)

SHL1STDLIBS += -framework AppKit -framework Carbon -framework Foundation

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

grammarcheck.mm: makeidl

ALLTAR : makeidl makeoxt

makeidl: XGrammarChecker.idl
	+idlc -C -I ../build/udkapi XGrammarChecker.idl
	+regmerge grammarcheck.uno.rdb /UCR XGrammarChecker.urd
	+cppumaker -BUCR -Torg.neooffice.XGrammarChecker ../build/offapi/type_reference/types.rdb grammarcheck.uno.rdb

makeoxt : $(SHL1TARGETN) META-INF/manifest.xml uiIntegration.xcu GrammarGUI/GrammarGUI.xdl GrammarGUI/XGrammarGUI.xba GrammarGUI/dialog.xlb GrammarGUI/script.xlb
	cp -f $(SHL1TARGETN)$ grammarcheck.uno.dylib
	zip grammarcheck.oxt META-INF/manifest.xml grammarcheck.uno.dylib grammarcheck.uno.rdb uiIntegration.xcu GrammarGUI/*
