#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.
#  
#  $RCSfile$
#  $Revision$
#  
#  This file is part of NeoOffice.
#  
#  NeoOffice is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 3
#  only, as published by the Free Software Foundation.
#  
#  NeoOffice is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License version 3 for more details
#  (a copy is included in the LICENSE file that accompanied this code).
#  
#  You should have received a copy of the GNU General Public License
#  version 3 along with NeoOffice.  If not, see
#  <http://www.gnu.org/licenses/gpl-3.0.txt>
#  for a copy of the GPLv3 License.
#  
#  Modified November 2014 by Patrick Luby. NeoOffice is distributed under
#  GPL only under Section 4 of the Apache License v2.0.
#  
#**************************************************************
PRJ=..$/..
PRJNAME=writerfilter
TARGET=rtftok
#LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
#USE_DEFFILE=TRUE
EXTERNAL_WARNINGS_NOT_ERRORS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/writerfilter.mk

#CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501 
#CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820


# --- Files --------------------------------------------------------

.IF "$(UPD)" == "310"
SLOFILES= \
	$(SLO)$/rtfcharsets.obj \
	$(SLO)$/rtfcontrolwords.obj \
	$(SLO)$/rtfdocumentfactory.obj \
	$(SLO)$/rtfdocumentimpl.obj \
	$(SLO)$/rtflookahead.obj \
	$(SLO)$/rtfreferenceproperties.obj \
	$(SLO)$/rtfreferencetable.obj \
	$(SLO)$/rtfsdrimport.obj \
	$(SLO)$/rtfskipdestination.obj \
	$(SLO)$/rtfsprm.obj \
	$(SLO)$/rtftokenizer.obj \
	$(SLO)$/rtfvalue.obj
.ELSE		# "$(UPD)" == "310"
SLOFILES=$(SLO)$/RTFScanner.obj $(SLO)$/RTFParseException.obj
.ENDIF		# "$(UPD)" == "310"


.IF "$(UPD)" != "310"
SHL1TARGET=$(TARGET)

SHL1STDLIBS=$(SALLIB)\
	$(CPPULIB)\
	$(CPPUHELPERLIB)
SHL1IMPLIB=i$(SHL1TARGET)
SHL1USE_EXPORTS=name

SHL1OBJS=$(SLOFILES) 

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)
.ENDIF		# "$(UPD)" != "310"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

.IF "$(UPD)" != "310"
RTFSCANNERCXX=$(MISC)/RTFScanner.cxx

GENERATEDFILES=$(RTFSCANNERCXX)

$(RTFSCANNERCXX): RTFScanner.lex RTFScanner.skl FlexLexer.h
	flex -+ -SRTFScanner.skl -o$@ RTFScanner.lex

$(SLO)/RTFScanner.obj: $(RTFSCANNERCXX)

.PHONY: genmake genclean

genmake: $(GENERATEDFILES)

genclean:
	rm $(GENERATEDFILES)
.ENDIF		# "$(UPD)" != "310"
