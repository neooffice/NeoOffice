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
#   Copyright 2008 by Planamesa Inc.
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

PRJNAME=neomobile
TARGET=neomobile.uno
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
	XNeoOfficeMobile.idl

UNOTYPES= \
	org.neooffice.XNeoOfficeMobile

# Force creation of the IDL header files before the compiling source files
UNOUCRDEP=$(OUT)$/ucr$/$(TARGET).db

SLOFILES= \
	$(SLO)$/NSWindow_Flipr.obj \
	$(SLO)$/neomobile.obj \
	$(SLO)$/neomobileappevent.obj \
	$(SLO)$/neomobileflipsideview.obj \
	$(SLO)$/neomobilei18n.obj \
	$(SLO)$/neomobilewebview.obj

SHL1TARGET=$(TARGET)
SHL1OBJS=$(SLOFILES) \
	$(SLO)$/neooffice.obj \
	$(SLO)$/neoofficeappevent.obj \
	$(SLO)$/neoofficei18n.obj
SHL1STDLIBS= \
	$(SALLIB) \
	$(CPPULIB) \
	$(CPPUHELPERLIB) \
	$(COMPHELPERLIB) \
	$(UNOTOOLSLIB) \
	$(VOSLIB)	\
	$(VCLLIB)

SHL1STDLIBS += -framework AppKit -framework QuartzCore -framework Security -framework WebKit

SHL2TARGET=$(TARGET:s/.uno//).office
SHL2OBJS=$(SLOFILES) \
	$(SLO)$/office.obj \
	$(SLO)$/officeappevent.obj \
	$(SLO)$/officei18n.obj
SHL2STDLIBS= \
	$(SALLIB)

SHL2STDLIBS += -framework AppKit -framework QuartzCore -framework Security -framework WebKit

# --- Targets ------------------------------------------------------

# Force zipping recipe to execute at the end
makeoxt : ALLTAR

.INCLUDE :  target.mk

$(MISC)$/%.xml : %.xml
	$(SED) 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' $< > $@

makeoxt : $(MISC)$/description.xml
	$(RM) $(BIN)$/$(PRJNAME).oxt
	zip $(ZIPFLAGS) $(PWD)$/$(BIN)$/$(PRJNAME).oxt $<
	zip -r $(BIN)$/$(PRJNAME).oxt META-INF NeoOfficeMobile uiIntegration.xcu uiJobs.xcu Images -x "*CVS*"
	zip $(ZIPFLAGS) $(PWD)$/$(BIN)$/$(PRJNAME).oxt $(UCR)$/$(TARGET).db -x "*CVS*"
	$(RM) -r $(LB)$/neo
	$(MKDIRHIER) $(LB)$/neo
	$(COPY) "$(LB)$/$(TARGET)$(DLLPOST)" "$(LB)$/neo$/$(TARGET)$(DLLPOST)"
# Change install names to avoid library loading issues
	sh -c -e 'install_name_tool -id "$(LB)$/neo$/$(TARGET)$(DLLPOST)" "$(LB)$/neo$/$(TARGET)$(DLLPOST)" ; for i in `otool -L "$(LB)$/neo$/$(TARGET)$(DLLPOST)" | awk "{ print \\$$1 }" | grep "^@loader_path\/"` ; do install_name_tool -change "$${i}" `echo "$${i}" | sed "s#^@loader_path/\.\./ure-link/lib/#@executable_path/urelibs/#" | sed "s#^@loader_path/#@executable_path/../basis-link/program/#"` "$(LB)$/neo$/$(TARGET)$(DLLPOST)" ; done'
# Change install names to avoid library loading issues. Note: basis-link/program
# directory is set to load in OOo's extension loader
	$(RM) -r $(LB)$/ooo
	$(MKDIRHIER) $(LB)$/ooo
	$(COPY) "$(LB)$/$(TARGET)$(DLLPOST)" "$(LB)$/ooo$/$(TARGET)$(DLLPOST)"
	sh -c -e 'install_name_tool -id "$(LB)$/ooo$/$(TARGET)$(DLLPOST)" "$(LB)$/ooo$/$(TARGET)$(DLLPOST)" ; for i in `otool -L "$(LB)$/ooo$/$(TARGET)$(DLLPOST)" | awk "{ print \\$$1 }" | grep "^@loader_path\/"` ; do install_name_tool -change "$${i}" `echo "$${i}" | sed "s#^@loader_path/\.\./ure-link/lib/#@executable_path/urelibs/#" | sed "s#^@loader_path/lib#@loader_path/../../../../../../basis-link/program/lib#"` "$(LB)$/ooo$/$(TARGET)$(DLLPOST)" ; done'
.IF "$(debug)" == ""
# Use stripped library if not in debug mode
	strip -S -x $(LB)$/neo$/$(TARGET)$(DLLPOST)
	strip -S -x $(LB)$/ooo$/$(TARGET)$(DLLPOST)
.ENDIF		# "$(debug)" == ""
	$(COPY) $(BIN)$/$(PRJNAME).oxt $(BIN)$/$(PRJNAME)-ooo.oxt
	zip $(ZIPFLAGS) $(PWD)$/$(BIN)$/$(PRJNAME).oxt $(LB)$/neo/$(TARGET)$(DLLPOST) -x "*CVS*"
	zip $(ZIPFLAGS) $(PWD)$/$(BIN)$/$(PRJNAME)-ooo.oxt $(LB)$/ooo$/$(TARGET)$(DLLPOST) -x "*CVS*"
