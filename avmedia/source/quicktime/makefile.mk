#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#  This file incorporates work covered by the following license notice:
# 
#    Modified February 2016 by Patrick Luby. NeoOffice is only distributed
#    under the GNU General Public License, Version 3 as allowed by Section 4
#    of the Apache License, Version 2.0.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#**************************************************************



PRJ=..$/..
PRJNAME=avmedia
.IF "$(GUIBASE)" == "java"
TARGET=avmediajava
.ELSE		# "$(GUIBASE)" == "java"
TARGET=avmediaQuickTime
.ENDIF		# "$(GUIBASE)" == "java"

# the QuickTime API has been deprecated since OSX 10.5 and has been removed in the OSX SDK 10.7
.IF "$(GUIBASE)" != "aqua" && "$(GUIBASE)" != "java"
dummy:
	@echo " Nothing to build for GUIBASE=$(GUIBASE) and OSX$(MACOSX_DEPLOYMENT_TARGET)"
.ELSE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

.IF "$(verbose)"!="" || "$(VERBOSE)"!=""
CDEFS+= -DVERBOSE
.ENDIF

# --- Files ----------------------------------

CFLAGSCXX+=$(OBJCXXFLAGS)

.IF "$(GUIBASE)" == "java"
SLOFILES= \
		$(SLO)$/quicktimeuno.obj  \
		$(SLO)$/quicktimecommon.obj \
		$(SLO)$/quicktimeframegrabber.obj \
		$(SLO)$/quicktimemanager.obj \
		$(SLO)$/quicktimeplayer.obj \
		$(SLO)$/quicktimewindow.obj
.ELSE		# "$(GUIBASE)" == "java"
SLOFILES= \
		$(SLO)$/quicktimeuno.obj  \
		$(SLO)$/framegrabber.obj        \
		$(SLO)$/manager.obj       \
		$(SLO)$/window.obj        \
		$(SLO)$/player.obj
.ENDIF		# "$(GUIBASE)" == "java"

.IF "$(GUIBASE)" == "java"
EXCEPTIONSFILES= \
		$(SLO)$/quicktimeuno.obj
.ELSE		# "$(GUIBASE)" == "java"
EXCEPTIONSFILES= \
		$(SLO)$/framegrabber.obj        \
		$(SLO)$/quicktimeuno.obj
.ENDIF		# "$(GUIBASE)" == "java"

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
             $(CPPULIB) \
             $(SALLIB)  \
             $(COMPHELPERLIB) \
             $(CPPUHELPERLIB) \
             $(TOOLSLIB) \
             $(VCLLIB) 

.IF "$(GUIBASE)" == "java"
SHL1STDLIBS+= \
             -framework AppKit
.ELSE		# "$(GUIBASE)" == "java"
SHL1STDLIBS+= \
             -framework Cocoa \
             -framework QTKit \
             -framework QuickTime
.ENDIF		# "$(GUIBASE)" == "java"

# build DLL
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1IMPLIB=i$(TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk

.ENDIF

.IF "$(GUIBASE)" == "java"

ALLTAR : $(MISC)/$(TARGET).component

$(MISC)/$(TARGET).component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt $(TARGET).component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt $(TARGET).component
.ELSE		# "$(GUIBASE)" == "java"

ALLTAR : $(MISC)/avmediaQuickTime.component

$(MISC)/avmediaQuickTime.component .ERRREMOVE : \
        $(SOLARENV)/bin/createcomponent.xslt avmediaQuickTime.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt avmediaQuickTime.component

.ENDIF		# "$(GUIBASE)" == "java"
