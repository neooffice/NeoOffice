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
PRJ=..$/..

PRJNAME=cpputools
TARGET=uno
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(GUIBASE)" == "java"
CDEFS+=-g
.ENDIF

.IF "$(OS)" == "MACOSX"
CDEFS+=-O0
.ENDIF

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb 
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

NO_OFFUH=TRUE
CPPUMAKERFLAGS+= -C

UNOTYPES= \
 	com.sun.star.uno.TypeClass \
 	com.sun.star.uno.XAggregation \
 	com.sun.star.uno.XWeak \
 	com.sun.star.uno.XComponentContext \
 	com.sun.star.lang.XTypeProvider \
	com.sun.star.lang.XMain \
 	com.sun.star.lang.XInitialization \
 	com.sun.star.lang.XComponent \
 	com.sun.star.lang.XSingleServiceFactory \
 	com.sun.star.lang.XSingleComponentFactory \
 	com.sun.star.lang.XMultiServiceFactory \
 	com.sun.star.lang.XMultiComponentFactory \
 	com.sun.star.container.XSet \
 	com.sun.star.container.XHierarchicalNameAccess \
	com.sun.star.loader.XImplementationLoader \
	com.sun.star.registry.XSimpleRegistry \
	com.sun.star.registry.XRegistryKey \
	com.sun.star.connection.XAcceptor \
	com.sun.star.connection.XConnection \
	com.sun.star.bridge.XBridgeFactory \
	com.sun.star.bridge.XBridge

# --- Files --------------------------------------------------------

DEPOBJFILES=$(OBJ)$/unoexe.obj

.IF "$(GUIBASE)" == "java"
DEPOBJFILES += $(OBJ)$/unoexe_cocoa.obj
.ENDIF		# "$(GUIBASE)" == "java"

APP1TARGET=$(TARGET)
APP1OBJS=$(DEPOBJFILES)  
APP1RPATH=UREBIN

# Include all relevant (see ure/source/README) dynamic libraries, so that C++
# UNO components running in the uno executable have a defined environment
# (stlport, unxlngi6 libstdc++.so.6, and wntmsci10 uwinapi.dll are already
# included via APP1STDLIB, unxlngi6 libgcc_s.so.1 and wntmsci10 msvcr71.dll and
# msvcp71.dll are magic---TODO):
APP1STDLIBS= \
	$(SALLIB)		\
    $(SALHELPERLIB) \
	$(CPPULIB)		\
	$(CPPUHELPERLIB)\
    $(LIBXML2LIB)
.IF "$(OS)" == "WNT"
APP1STDLIBS += $(UNICOWSLIB)
.ENDIF

.IF "$(GUIBASE)" == "java"
APP1STDLIBS += -framework AppKit
.ENDIF		# "$(GUIBASE)" == "java"

.INCLUDE :  target.mk

