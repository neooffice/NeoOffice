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

PRJNAME=mediabrowser
TARGET=mediabrowser.uno
TARGETTYPE=CUI

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CDEFS+=-DDLLPOSTFIX=$(DLLPOSTFIX)

# Don't put lib prefix on shared library
DLLPRE=

# Add locally built types registry to cppumaker search path
UNOUCRRDB+=$(OUT)$/ucr$/$(TARGET).db

# --- Files --------------------------------------------------------

IDLFILES= \
	XMediaBrowser.idl

UNOTYPES= \
	org.neooffice.XMediaBrowser

# Force creation of the IDL header files before the compiling source files
UNOUCRDEP=$(OUT)$/ucr$/$(TARGET).db

SLOFILES= \
	$(SLO)$/mediabrowser.obj

SHL1TARGET=$(TARGET)
SHL1OBJS=$(SLOFILES)
SHL1STDLIBS= \
	$(SALLIB) \
	$(CPPULIB) \
	$(CPPUHELPERLIB) \
	$(COMPHELPERLIB) \
	$(VOSLIB)	\
	$(VCLLIB)

SHL1STDLIBS += -framework AppKit -framework Carbon -framework Foundation 

# --- Targets ------------------------------------------------------

# Force zipping recipe to execute at the end
makeoxt : ALLTAR

.INCLUDE :  target.mk

makeoxt :
	$(RM) $(BIN)$/$(PRJNAME).oxt
	zip -r $(BIN)$/$(PRJNAME).oxt META-INF MediaBrowser uiIntegration.xcu -x "*CVS*"
	zip $(ZIPFLAGS) $(PWD)$/$(BIN)$/$(PRJNAME).oxt $(UCR)$/$(TARGET).db -x "*CVS*"
.IF "$(debug)" == ""
# Use stripped library if not in debug mode
	$(RM) $(BIN)$/stripped
	$(MKDIRHIER) $(LB)$/stripped
	$(COPY) $(LB)$/$(TARGET)$(DLLPOST) $(LB)$/stripped$/$(TARGET)$(DLLPOST)
	strip -S -x $(LB)$/stripped$/$(TARGET)$(DLLPOST)
	zip $(ZIPFLAGS) $(PWD)$/$(BIN)$/$(PRJNAME).oxt $(LB)$/stripped/$(TARGET)$(DLLPOST) -x "*CVS*"
.ELSE		# "$(debug)" == ""
	zip $(ZIPFLAGS) $(PWD)$/$(BIN)$/$(PRJNAME).oxt $(LB)$/$(TARGET)$(DLLPOST) -x "*CVS*"
.ENDIF		# "$(debug)" == ""
