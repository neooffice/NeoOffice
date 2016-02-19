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



PRJ=..

PRJNAME=desktop
TARGET=soffice
TARGETTYPE=GUI
LIBTARGET=NO
GEN_HID=TRUE
GEN_HID_OTHER=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

UWINAPILIB =

VERINFONAME=verinfo

# --- Resourcen ----------------------------------------------------

.IF "$(GUI)" == "WNT"
RCFILES=verinfo.rc
.ENDIF
.IF "$(GUI)" == "OS2"
RCFILES=ooverinfo2.rc
.ENDIF

# --- Linken der Applikation ---------------------------------------

.IF "$(OS)" == "MACOSX"
LINKFLAGSAPPGUI!:=	$(LINKFLAGSAPPGUI:s/-bind_at_load//)
.ENDIF # MACOSX

.IF "$(GUIBASE)" == "java"
JAVAAPPOBJS = $(OBJ)$/main_java.obj
JAVAAPPSTDLIBS = -Wl,-rpath,@executable_path/../basis-link/program -Wl,-rpath,@executable_path/../basis-link/ure-link/lib -Wl,-rpath,/usr/lib -Wl,-rpath,/usr/local/lib -framework AppKit
.ENDIF		# "$(GUIBASE)" == "java"

#.IF "$(OS)" == "LINUX" || "$(OS)" == "FREEBSD" || "$(OS)" == "NETBSD"
## #74158# linux needs sal/vos/tools at end of link list, solaris needs it first,
## winXX is handled like solaris for now
#APP1_STDPRE=
#APP1_STDPOST=$(CPPULIB) $(CPPUHELPERLIB) $(UNOLIB) $(TOOLSLIB) \
#	$(VOSLIB) $(SALLIB)
#.ELSE
#APP1_STDPRE=$(SALLIB) $(VOSLIB) $(TOOLSLIB) $(UNOLIB) $(CPPULIB) \
#	$(CPPUHELPERLIB)
#APP1_STDPOST=
#.ENDIF

RESLIB1NAME=		dkt
RESLIB1IMAGES=		$(PRJ)$/res
RESLIB1SRSFILES=	$(SRS)$/desktop.srs \
                    $(SRS)$/wizard.srs

.IF "$(L10N_framework)"==""
.IF "$(LINK_SO)"=="TRUE"
.IF "$(GUI)" != "OS2"
APP1TARGET=so$/$(TARGET)
APP1NOSAL=TRUE
APP1RPATH=BRAND
APP1OBJS=$(OBJ)$/copyright_ascii_sun.obj $(OBJ)$/main.obj
.IF "$(GUIBASE)" == "java"
APP1OBJS += \
	$(JAVAAPPOBJS) \
	$(OBJ)$/main_java_init.obj
APP1STDLIBS = $(JAVAAPPSTDLIBS)
.ELSE		# "$(GUIBASE)" == "java"
APP1STDLIBS = $(SALLIB) $(SOFFICELIB)
.ENDIF		# "$(GUIBASE)" == "java"
APP1DEPN= $(APP1RES) verinfo.rc

.IF "$(GUI)" == "WNT"
APP1RES=    $(RES)$/desktop.res
APP1ICON=$(SOLARRESDIR)$/icons/so9_main_app.ico
APP1VERINFO=verinfo.rc
APP1LINKRES=$(MISC)$/$(TARGET)1.res
APP1STACK=10000000

# create a manifest file with the same name as the
#office executable file soffice.exe.manifest
#$(BIN)$/$(TARGET).exe.manifest: template.manifest
#$(COPY) $< $@

.ENDIF # WNT

.ENDIF # "$(GUI)" != "OS2"

.ENDIF # "$(LINK_SO)"=="TRUE"

APP5TARGET=soffice
APP5NOSAL=TRUE
APP5RPATH=BRAND
APP5OBJS=$(OBJ)$/copyright_ascii_ooo.obj $(OBJ)$/main.obj
.IF "$(GUIBASE)" == "java"
APP5OBJS += \
	$(JAVAAPPOBJS) \
	$(OBJ)$/main_java_init.obj
APP5STDLIBS = $(JAVAAPPSTDLIBS)
.ELSE		# "$(GUIBASE)" == "java"
APP5STDLIBS = $(SALLIB) $(SOFFICELIB)
.ENDIF		# "$(GUIBASE)" == "java"
.IF "$(OS)" == "LINUX"
APP5STDLIBS+= -lXext -lX11
#APP5STDLIBS+= -lXext -lSM -lICE
.ENDIF # LINUX

APP5DEPN= $(APP1TARGETN) $(APP5RES) ooverinfo.rc
APP5DEF=    $(MISCX)$/$(TARGET).def

.IF "$(GUI)" == "WNT"
APP5RES=    $(RES)$/oodesktop.res
APP5ICON=$(SOLARRESDIR)$/icons/ooo3_main_app.ico
APP5VERINFO=ooverinfo.rc
APP5LINKRES=$(MISC)$/ooffice5.res
APP5STACK=10000000
.ENDIF # WNT

.IF "$(GUI)" == "OS2"
APP5DEF= # automatic
APP5RES=    $(RES)$/oodesktop.res
APP5ICON=$(SOLARRESDIR)$/icons/ooo-main-app.ico
APP5VERINFO=ooverinfo2.rc
APP5LINKRES=$(MISC)$/ooffice.res
.ENDIF # OS2

.IF "$(GUI)" == "WNT"
.IF "$(LINK_SO)"=="TRUE"
APP6TARGET=so$/officeloader
APP6RES=$(RES)$/soloader.res
APP6NOSAL=TRUE
APP6DEPN= $(APP1TARGETN) $(APP6RES) verinfo.rc
APP6VERINFO=verinfo.rc
APP6LINKRES=$(MISC)$/soffice6.res
APP6ICON=$(SOLARRESDIR)$/icons/so9_main_app.ico
APP6OBJS = \
    $(OBJ)$/extendloaderenvironment.obj \
    $(OBJ)$/officeloader.obj \
    $(SOLARLIBDIR)$/pathutils-obj.obj
STDLIB6=$(ADVAPI32LIB) $(SHELL32LIB) $(SHLWAPILIB)
.ENDIF # "$(LINK_SO)"=="TRUE"

APP7TARGET=officeloader
APP7RES=$(RES)$/ooloader.res
APP7NOSAL=TRUE
APP7DEPN= $(APP1TARGETN) $(APP7RES) ooverinfo.rc
APP7VERINFO=ooverinfo.rc
APP7LINKRES=$(MISC)$/ooffice7.res
APP7ICON=$(SOLARRESDIR)$/icons/ooo3_main_app.ico
APP7OBJS = \
    $(OBJ)$/extendloaderenvironment.obj \
    $(OBJ)$/officeloader.obj \
    $(SOLARLIBDIR)$/pathutils-obj.obj
STDLIB7=$(ADVAPI32LIB) $(SHELL32LIB) $(SHLWAPILIB)
.ELIF "$(OS)" == "MACOSX"
APP6TARGET=officeloader
APP6NOSAL=TRUE
APP6RPATH=BRAND
APP6OBJS=$(OBJ)$/copyright_ascii_ooo.obj $(OBJ)$/officeloader.obj
APP6STDLIBS = $(SALLIB)
APP5DEPN= $(APP1TARGETN) $(APP5RES) ooverinfo.rc
APP5DEF=    $(MISCX)$/$(TARGET).def
.ENDIF # WNT

.IF "$(PRODUCT_BUILD_TYPE)" == "java"

APP8TARGET=soffice2
APP8NOSAL=TRUE
APP8RPATH=BRAND
APP8OBJS=$(OBJ)$/copyright_ascii_ooo.obj $(OBJ)$/main.obj
.IF "$(GUIBASE)" == "java"
APP8OBJS += \
	$(JAVAAPPOBJS) \
	$(OBJ)$/main_java_init2.obj
APP8STDLIBS = $(JAVAAPPSTDLIBS)
.ELSE		# "$(GUIBASE)" == "java"
APP8STDLIBS = $(SALLIB) $(SOFFICELIB)
.ENDIF		# "$(GUIBASE)" == "java"

APP8DEPN= $(APP1TARGETN) $(APP8RES) ooverinfo.rc
APP8DEF=    $(MISCX)$/$(TARGET).def

.IF "$(GUI)" == "WNT"
APP8RES=    $(RES)$/oodesktop.res
APP8ICON=$(SOLARRESDIR)$/icons/ooo3_main_app.ico
APP8VERINFO=ooverinfo.rc
APP8LINKRES=$(MISC)$/ooffice5.res
APP8STACK=10000000
.ENDIF # WNT

APP9TARGET=soffice3
APP9NOSAL=TRUE
APP9RPATH=BRAND
APP9OBJS=$(OBJ)$/copyright_ascii_ooo.obj $(OBJ)$/main.obj
.IF "$(GUIBASE)" == "java"
APP9OBJS += \
	$(JAVAAPPOBJS) \
	$(OBJ)$/main_java_init3.obj
APP9STDLIBS = $(JAVAAPPSTDLIBS)
.ELSE		# "$(GUIBASE)" == "java"
APP9STDLIBS = $(SALLIB) $(SOFFICELIB)
.ENDIF		# "$(GUIBASE)" == "java"

APP9DEPN= $(APP1TARGETN) $(APP9RES) ooverinfo.rc
APP9DEF=    $(MISCX)$/$(TARGET).def

.IF "$(GUI)" == "WNT"
APP9RES=    $(RES)$/oodesktop.res
APP9ICON=$(SOLARRESDIR)$/icons/ooo3_main_app.ico
APP9VERINFO=ooverinfo.rc
APP9LINKRES=$(MISC)$/ooffice5.res
APP9STACK=10000000
.ENDIF # WNT

.ENDIF		# "$(PRODUCT_BUILD_TYPE)" == "java"

.ENDIF

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

.IF "$(L10N_framework)"==""

.IF "$(APP1TARGETN)"!=""
$(APP1TARGETN) :  $(MISC)$/binso_created.flg
.ENDIF			# "$(APP1TARGETN)"!=""

.IF "$(APP5TARGETN)"!=""
$(APP5TARGETN) :  $(MISC)$/binso_created.flg
.ENDIF			# "$(APP6TARGETN)"!=""

.IF "$(APP6TARGETN)"!=""
$(APP6TARGETN) :  $(MISC)$/binso_created.flg
.ENDIF			# "$(APP6TARGETN)"!=""

.IF "$(GUI)" == "WNT"
ALLTAR: $(MISC)$/$(TARGET).exe.manifest
ALLTAR: $(MISC)$/$(TARGET).bin.manifest
ALLTAR: $(BIN)$/$(TARGET).bin
.IF "$(LINK_SO)"=="TRUE"
ALLTAR: $(BIN)$/so$/$(TARGET).bin
.ENDIF # "$(LINK_SO)"=="TRUE"
.ENDIF # WNT

.IF "$(GUI)" == "OS2"
ALLTAR: $(BIN)$/$(TARGET).bin
.ENDIF # OS2

$(BIN)$/soffice_oo$(EXECPOST) : $(APP5TARGETN)
	$(COPY) $< $@

.IF "$(GUI)" != "OS2"
.IF "$(LINK_SO)"=="TRUE"
$(BIN)$/so$/soffice_so$(EXECPOST) : $(APP1TARGETN)
	$(COPY) $< $@

ALLTAR : $(BIN)$/so$/soffice_so$(EXECPOST)
.ENDIF # "$(LINK_SO)"=="TRUE"
ALLTAR : $(BIN)$/soffice_oo$(EXECPOST)
.ENDIF

.IF "$(OS)" == "MACOSX"
.IF "$(LINK_SO)"=="TRUE"
$(BIN)$/so$/soffice_mac$(EXECPOST) : $(APP1TARGETN)
	$(COPY) $< $@
	
ALLTAR : $(BIN)$/so$/soffice_mac$(EXECPOST)
.ENDIF # "$(LINK_SO)"=="TRUE"

$(BIN)$/soffice_mac$(EXECPOST) : $(APP5TARGETN)
	$(COPY) $< $@

ALLTAR : $(BIN)$/soffice_mac$(EXECPOST)

.ENDIF # "$(OS)" == "MACOSX"

.IF "$(GUI)" == "WNT"

# create a manifest file with the same name as the
# office executable file soffice.exe.manifest
.IF "$(CCNUMVER)" <= "001399999999"
$(MISC)$/$(TARGET).exe.manifest: template.manifest
   $(COPY) $< $@
.ELSE
$(MISC)$/$(TARGET).exe.template.manifest: template.manifest
   $(COPY) $< $@

$(MISC)$/$(TARGET).exe.linker.manifest: $(BIN)$/$(TARGET)$(EXECPOST)
   mt.exe -inputresource:$(BIN)$/$(TARGET)$(EXECPOST) -out:$@

$(MISC)$/$(TARGET).exe.manifest: $(MISC)$/$(TARGET).exe.template.manifest $(MISC)$/$(TARGET).exe.linker.manifest
   mt.exe -manifest $(MISC)$/$(TARGET).exe.linker.manifest $(MISC)$/$(TARGET).exe.template.manifest -out:$@
.ENDIF

# create a manifest file with the same name as the
# office executable file soffice.bin.manifest
.IF "$(CCNUMVER)" <= "001399999999"
$(MISC)$/$(TARGET).bin.manifest: template.manifest
   $(COPY) $< $@
.ELSE
$(MISC)$/$(TARGET).bin.manifest: $(MISC)$/$(TARGET).exe.manifest
   $(COPY) $(MISC)$/$(TARGET).exe.manifest $@
.ENDIF

$(BIN)$/$(TARGET).bin: $(BIN)$/$(TARGET)$(EXECPOST)
   $(COPY) $< $@

$(BIN)$/so$/$(TARGET).bin: $(BIN)$/so$/$(TARGET)$(EXECPOST)
   $(COPY) $< $@

.ENDIF # WNT

.IF "$(GUI)" == "OS2"
$(BIN)$/$(TARGET).bin: $(BIN)$/$(TARGET)$(EXECPOST)
   $(COPY) $< $@
.ENDIF # OS2

$(MISC)$/binso_created.flg :
	@@-$(MKDIRHIER) $(BIN)$/so && $(TOUCH) $@

.ENDIF
