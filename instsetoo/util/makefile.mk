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
#   Sun Microsystems Inc., October, 2000
# 
#   GNU General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
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
#   =================================================
#   Modified June 2004 by Patrick Luby. SISSL Removed. NeoOffice is
#   distributed under GPL only under modification term 3 of the LGPL.
# 
#   Contributor(s): _______________________________________
# 
##########################################################################

PRJ=..
PRJNAME=OpenOffice
TARGET=util

.INCLUDE:  settings.mk

.IF "$(BUILD_SPECIAL)"!=""
LZIPFLAGS*:=-S
.ENDIF

.IF "$(strip)"!=""
LZIPFLAGS*:=-S
.ENDIF

.IF "$(OS)"=="WNT"
EXTRARMFLAG=/S
.ELSE
EXTRARMFLAG=-r
.ENDIF

LZIPFLAGS+=-e $(MISC)$/lzip.log
SHARED_COM_SDK_PATH*:=.

INSTALLDIR=$(OUT)

SEARCH_DIR:=.
.IF "$(BUILD_SPECIAL)"!=""
.IF "$(GUI)"=="WNT"
SEARCH_DIR=r:\solenv\inst\ooo\$(OUTPATH)
.ENDIF
.ENDIF

.INCLUDE: target.mk

.IF "$(BSCLIENT)"==""

ALLTAR : pack

LANGUAGES = $(alllangext:s/ /,/)

.IF "$(alllangext)"!=""

pack:
	+-$(RM) $(EXTRARMFLAG) $(INSTALLDIR)$/$(LANGUAGES)$/normal$/*
	+-$(LZIP) -p ${SEARCH_DIR} $(LZIPFLAGS) -l $(LANGUAGES) -f openoffice.lst -d $(INSTALLDIR)$/$(LANGUAGES) -n OfficeOSL -e $(INSTALLDIR)$/$(LANGUAGES)$/Logfile.txt -C $(INSTALLDIR)$/$(LANGUAGES)$/checksums.txt

test:
	+-$(LZIP) $(LZIPFLAGS) -p ${SEARCH_DIR} -l 01 -f openoffice.lst -o -n OfficeOSL 

.ELSE			# "$(alllangext)"!=""
pack:
	@+echo cannot pack nothing...

.ENDIF			# "$(alllangext)"!=""

# Special target to echo the $(alllangext) macro to external scripts
language_numbers:
	@echo $(LANGUAGES)

# Special target to echo the $(alllangext) macro to external scripts
language_names:
	@echo $(foreach,i,$(alllangext) $(iso_$i))

.ENDIF          # "$(BSCLIENT)"==""
	
