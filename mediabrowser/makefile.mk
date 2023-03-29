# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

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

SHL1STDLIBS += -framework AppKit

# --- Targets ------------------------------------------------------

# Force zipping recipe to execute at the end
makeoxt : ALLTAR

.INCLUDE :  target.mk

$(MISC)$/%.xml : %.xml
	$(SED) 's#$$(PRODUCT_NAME)#$(PRODUCT_NAME)#g' $< > $@

makeoxt : $(MISC)$/description.xml
	$(RM) $(BIN)$/$(PRJNAME).oxt
	zip $(ZIPFLAGS) $(PWD)$/$(BIN)$/$(PRJNAME).oxt $<
	zip -r $(BIN)$/$(PRJNAME).oxt META-INF MediaBrowser uiIntegration.xcu uiJobs.xcu Images -x "*CVS*"
	zip $(ZIPFLAGS) $(PWD)$/$(BIN)$/$(PRJNAME).oxt $(UCR)$/$(TARGET).db -x "*CVS*"
# Change install names to avoid library loading issues
	sh -c -e 'install_name_tool -id "$(LB)$/$(TARGET)$(DLLPOST)" "$(LB)$/$(TARGET)$(DLLPOST)" ; for i in `otool -L "$(LB)$/$(TARGET)$(DLLPOST)" | awk "{ print \\$$1 }" | grep "^@loader_path\/"` ; do install_name_tool -change "$${i}" `echo "$${i}" | sed "s#^@loader_path/\.\./ure-link/lib/#@executable_path/urelibs/#" | sed "s#^@loader_path/#@executable_path/../basis-link/program/#"` "$(LB)$/$(TARGET)$(DLLPOST)" ; done'
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
