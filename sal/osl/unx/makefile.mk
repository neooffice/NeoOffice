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
# Modified December 2005 by Patrick Luby. NeoOffice is distributed under
# GPL only under modification term 2 of the LGPL.
#
#*************************************************************************

PRJ=..$/..

PRJNAME=sal
.IF "$(WORK_STAMP)"=="MIX364"
TARGET=cppsal
.ELSE
TARGET=cpposl
.ENDIF
USE_LDUMP2=TRUE

PROJECTPCH4DLL=TRUE
PROJECTPCH=cont_pch
PROJECTPCHSOURCE=cont_pch

TARGETTYPE=CUI

.IF "$(PRODUCT_FILETYPE)"!=""
ENVCDEFS += -DPRODUCT_FILETYPE=\'$(PRODUCT_FILETYPE)\'
.ENDIF      # "$(PRODUCT_FILETYPE)"!=""

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

SLOFILES=   $(SLO)$/conditn.obj  \
			$(SLO)$/diagnose.obj \
			$(SLO)$/semaphor.obj \
			$(SLO)$/socket.obj   \
			$(SLO)$/interlck.obj \
			$(SLO)$/mutex.obj    \
			$(SLO)$/nlsupport.obj \
			$(SLO)$/thread.obj   \
			$(SLO)$/module.obj   \
			$(SLO)$/process.obj  \
			$(SLO)$/security.obj \
			$(SLO)$/profile.obj  \
			$(SLO)$/time.obj     \
			$(SLO)$/file.obj     \
			$(SLO)$/signal.obj   \
			$(SLO)$/pipe.obj   	 \
			$(SLO)$/system.obj	 \
			$(SLO)$/util.obj	 \
			$(SLO)$/tempfile.obj\
			$(SLO)$/file_url.obj\
			$(SLO)$/file_error_transl.obj\
			$(SLO)$/file_path_helper.obj\
			$(SLO)$/uunxapi.obj\
			$(SLO)$/process_impl.obj\
			$(SLO)$/file_stat.obj \
			$(SLO)$/salinit.obj

#.IF "$(UPDATER)"=="YES"
OBJFILES=   $(OBJ)$/conditn.obj  \
			$(OBJ)$/diagnose.obj \
			$(OBJ)$/semaphor.obj \
			$(OBJ)$/socket.obj   \
			$(OBJ)$/interlck.obj \
			$(OBJ)$/mutex.obj    \
			$(OBJ)$/nlsupport.obj \
			$(OBJ)$/thread.obj   \
			$(OBJ)$/module.obj   \
			$(OBJ)$/process.obj  \
			$(OBJ)$/security.obj \
			$(OBJ)$/profile.obj  \
			$(OBJ)$/time.obj     \
			$(OBJ)$/file.obj     \
			$(OBJ)$/signal.obj   \
			$(OBJ)$/pipe.obj   	 \
			$(OBJ)$/system.obj	 \
			$(OBJ)$/util.obj	 \
			$(OBJ)$/tempfile.obj\
			$(OBJ)$/file_url.obj\
			$(OBJ)$/file_error_transl.obj\
			$(OBJ)$/file_path_helper.obj\
			$(OBJ)$/uunxapi.obj\
			$(OBJ)$/process_impl.obj\
			$(OBJ)$/file_stat.obj \
			$(OBJ)$/salinit.obj
			
#.ENDIF

.IF "$(OS)"=="MACOSX"
SLOFILES += $(SLO)$/osxlocale.obj
.ENDIF

.IF "$(OS)"=="SOLARIS" || "$(OS)"=="FREEBSD" || "$(OS)"=="NETBSD" || "$(OS)$(CPU)"=="LINUXS" || "$(OS)"=="MACOSX"
SLOFILES += $(SLO)$/backtrace.obj
OBJFILES += $(OBJ)$/backtrace.obj
.ENDIF

.IF "$(GUIBASE)"=="java"
SLOFILES += $(SLO)$/pipe_ports.obj $(SLO)$/system_cocoa.obj
OBJFILES += $(OBJ)$/pipe_ports.obj $(OBJ)$/system_cocoa.obj
.ENDIF		# "$(GUIBASE)"=="java"

# --- Targets ------------------------------------------------------

.IF "$(COM)"=="C50"
APP1STDLIBS+=-lC
.ENDIF

.IF "$(OS)" == "LINUX"
.IF "$(PAM)" == "NO"
CFLAGS+=-DNOPAM
.IF "$(NEW_SHADOW_API)" == "YES"
CFLAGS+=-DNEW_SHADOW_API
.ENDIF
.ENDIF
.IF "$(PAM_LINK)" == "YES"
CFLAGS+=-DPAM_LINK
.ENDIF
.IF "$(CRYPT_LINK)" == "YES"
CFLAGS+=-DCRYPT_LINK
.ENDIF
.ENDIF

.IF "$(ENABLE_CRASHDUMP)" != "" || "$(PRODUCT)" == ""
CFLAGS+=-DSAL_ENABLE_CRASH_REPORT
.ENDIF

.INCLUDE :  target.mk

.IF "$(OS)$(CPU)"=="SOLARISU" || "$(OS)$(CPU)"=="SOLARISS" || "$(OS)$(CPU)"=="NETBSDS" || "$(OS)$(CPU)"=="LINUXS"

$(SLO)$/interlck.obj: $(SLO)$/interlck.o
	 touch $(SLO)$/interlck.obj

$(OBJ)$/interlck.obj: $(OBJ)$/interlck.o
	 touch $(OBJ)$/interlck.obj

$(SLO)$/interlck.o: $(MISC)$/interlck_sparc.s
	$(ASM) $(AFLAGS) -o $@ $<

$(OBJ)$/interlck.o: $(MISC)$/interlck_sparc.s
	$(ASM) $(AFLAGS) -o $@ $<

$(MISC)$/interlck_sparc.s: asm/interlck_sparc.s
	tr -d "\015" < $< > $@

.ENDIF

.IF "$(OS)$(CPU)"=="SOLARISI"

$(SLO)$/interlck.obj: $(SLO)$/interlck.o
	touch $(SLO)$/interlck.obj

$(OBJ)$/interlck.obj: $(OBJ)$/interlck.o
	touch $(OBJ)$/interlck.obj

$(SLO)$/interlck.o: $(MISC)$/interlck_x86.s
	$(ASM) $(AFLAGS) -o $@ $<

$(OBJ)$/interlck.o: $(MISC)$/interlck_x86.s
	$(ASM) $(AFLAGS) -o $@ $<

$(MISC)$/interlck_x86.s: asm/interlck_x86.s
	tr -d "\015" < $< > $@

.ENDIF
