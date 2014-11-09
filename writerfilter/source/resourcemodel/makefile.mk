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
TARGET=resourcemodel
#LIBTARGET=NO
#USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/writerfilter.mk

#CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
#CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS+=-DWRITERFILTER_DLLIMPLEMENTATION


# --- Files --------------------------------------------------------

# work around gcc taking hours and/or OOM'ing on this file
NOOPTFILES= \
	$(SLO)$/qnametostr.obj

SLOFILES= \
	$(SLO)$/Fraction.obj \
	$(SLO)$/LoggedResources.obj \
	$(SLO)$/ResourceModelHelper.obj \
	$(SLO)$/TagLogger.obj \
	$(SLO)$/XPathLogger.obj \
	$(SLO)$/qnametostr.obj \
	$(SLO)$/util.obj

.IF "$(UPD)" == "310"
SLOFILES += \
	$(SLO)$/qnametostrcore.obj
.ELSE		# "$(UPD)" == "310"
SLOFILES += \
	$(SLO)$/Protocol.obj \
	$(SLO)$/WW8Analyzer.obj \
	$(SLO)$/resourcemodel.obj \
	$(SLO)$/sprmcodetostr.obj
.ENDIF		# "$(UPD)" == "310"

# FreeBSD/Linux 64-bit: compiler (gcc 4.2.x) fails with 'out of memory'
.IF "$(OUTPATH)"=="unxfbsdx" || "$(OUTPATH)"=="unxfbsdi" || "$(OUTPATH)"=="unxlngx6"
NOOPTFILES= \
	$(SLO)$/qnametostr.obj
.ENDIF

.IF "$(UPD)" != "310"
SHL1TARGET=$(TARGET)

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
RTFTOKLIB=-lrtftok
DOCTOKLIB=-ldoctok
OOXMLLIB=-looxml
.ELIF "$(GUI)"=="WNT"
RTFTOKLIB=$(LB)$/irtftok.lib
DOCTOKLIB=$(LB)$/idoctok.lib
OOXMLLIB=$(LB)$/iooxml.lib
.ENDIF

SHL1STDLIBS=$(SALLIB)\
	$(CPPULIB)\
	$(CPPUHELPERLIB) \
    $(COMPHELPERLIB)

SHL1IMPLIB=i$(SHL1TARGET)
SHL1USE_EXPORTS=name

SHL1OBJS=$(SLOFILES)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEFLIB1NAME=$(TARGET)
.ENDIF		# "$(UPD)" != "310"

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

RESOURCEMODELCXXOUTDIR=$(MISC)
DOCTOKHXXOUTDIR=$(INCCOM)$/doctok
OOXMLHXXOUTDIR=$(INCCOM)$/ooxml

DOCTOKHXXOUTDIRCREATED=$(DOCTOKHXXOUTDIR)$/created
OOXMLHXXOUTDIRCREATED=$(OOXMLHXXOUTDIR)$/created

OOXMLMODEL=..$/ooxml$/model.xml
OOXMLPREPROCESSXSL=..$/ooxml$/modelpreprocess.xsl
OOXMLPREPROCESSXSLCOPIED=$(MISC)$/modelpreprocess.xsl
OOXMLQNAMETOSTRXSL=..$/ooxml$/qnametostr.xsl
OOXMLANALYZERXSL=..$/ooxml$/analyzer.xsl
OOXMLRESOURCEIDSXSL=..$/ooxml$/resourceids.xsl
OOXMLFACTORYTOOLSXSL=..$/ooxml$/factorytools.xsl
DOCTOKMODEL=..$/doctok$/resources.xmi
DOCTOKQNAMETOSTRXSL=..$/doctok$/qnametostr.xsl
DOCTOKANALYZERXSL=..$/doctok$/analyzer.xsl
DOCTOKSPRMCODETOSTRXSL=..$/doctok$/sprmcodetostr.xsl
DOCTOKRESOURCEIDSXSL=..$/doctok$/resourceids.xsl
DOCTOKSPRMIDSXSL=..$/doctok$/sprmids.xsl
DOCTOKRESOURCETOOLS=..$/doctok$/resourcetools.xsl

NSPROCESS=namespace_preprocess.pl

MODELPROCESSED=$(MISC)$/model_preprocessed.xml

QNAMETOSTRCXX=$(RESOURCEMODELCXXOUTDIR)$/qnametostr.cxx
OOXMLQNAMETOSTRTMP=$(RESOURCEMODELCXXOUTDIR)$/OOXMLqnameToStr.tmp
DOCTOKQNAMETOSTRTMP=$(RESOURCEMODELCXXOUTDIR)$/DOCTOKqnameToStr.tmp
SPRMCODETOSTRCXX=$(RESOURCEMODELCXXOUTDIR)$/sprmcodetostr.cxx
SPRMCODETOSTRTMP=$(RESOURCEMODELCXXOUTDIR)$/sprmcodetostr.tmp
DOCTOKRESOURCEIDSHXX=$(DOCTOKHXXOUTDIR)$/resourceids.hxx
SPRMIDSHXX=$(DOCTOKHXXOUTDIR)$/sprmids.hxx
OOXMLRESOURCEIDSHXX=$(OOXMLHXXOUTDIR)$/resourceids.hxx

NSXSL=$(MISC)$/namespacesmap.xsl
.IF "$(UPD)" == "310"
NAMESPACESTXT=$(PRJ)$/..$/oox$/$(INPATH)$/misc/namespaces.txt
.ELSE		# "$(UPD)" == "310"
NAMESPACESTXT=$(SOLARVER)$/$(INPATH)$/inc$(UPDMINOREXT)$/oox$/token$/namespaces.txt
.ENDIF		# "$(UPD)" == "310"

GENERATEDHEADERS=$(OOXMLRESOURCEIDSHXX)
GENERATEDFILES= \
	$(GENERATEDHEADERS) \
	$(QNAMETOSTRCXX) \
	$(MODELPROCESSED) \

.IF "$(UPD)" != "310"
GENERATEDHEADERS += $(DOCTOKRESOURCEIDSHXX) $(SPRMIDSHXX)
GENERATEDFILES += \
	$(SPRMCODETOSTRCXX) \
	$(OOXMLQNAMETOSTRTMP) \
	$(DOCTOKQNAMETOSTRTMP) \
	$(SPRMCODETOSTRTMP)
.ENDIF		# "$(UPD)" != "310"

.IF "$(UPD)" == "310"
$(QNAMETOSTRCXX): $(OOXMLQNAMETOSTRXSL) $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
	$(XSLTPROC) $(OOXMLQNAMETOSTRXSL:s!\!/!) $(MODELPROCESSED) > $@
.ELSE		# "$(UPD)" == "310"
$(OOXMLQNAMETOSTRTMP): $(OOXMLQNAMETOSTRXSL) $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
	$(XSLTPROC) $(OOXMLQNAMETOSTRXSL:s!\!/!) $(MODELPROCESSED) > $@
.ENDIF		# "$(UPD)" == "310"

$(DOCTOKQNAMETOSTRTMP): $(DOCTOKQNAMETOSTRXSL) $(DOCTOKMODEL)
    @echo "Making:   " $(@:f)   
	$(XSLTPROC) $(DOCTOKQNAMETOSTRXSL:s!\!/!) $(DOCTOKMODEL) > $@

.IF "$(UPD)" != "310"
$(QNAMETOSTRCXX): $(OOXMLQNAMETOSTRTMP) $(DOCTOKQNAMETOSTRTMP) qnametostrheader qnametostrfooter $(OOXMLFACTORYTOOLSXSL) $(DOCTOKRESOURCETOOLS)
	@$(TYPE) qnametostrheader $(OOXMLQNAMETOSTRTMP) $(DOCTOKQNAMETOSTRTMP) qnametostrfooter > $@
.ENDIF		# "$(UPD)" != "310"

$(SPRMCODETOSTRTMP): $(DOCTOKSPRMCODETOSTRXSL) $(DOCTOKMODEL)
    @echo "Making:   " $(@:f)   
	$(XSLTPROC) $(DOCTOKSPRMCODETOSTRXSL:s!\!/!) $(DOCTOKMODEL) > $@

$(SPRMCODETOSTRCXX): sprmcodetostrheader $(SPRMCODETOSTRTMP) sprmcodetostrfooter
	@$(TYPE) $< > $@

$(SLO)$/sprmcodetostr.obj: $(SPRMCODETOSTRCXX)
$(SLO)$/qnametostr.obj: $(QNAMETOSTRCXX)

$(SLOFILES): $(GENERATEDHEADERS)

$(DOCTOKHXXOUTDIRCREATED):
	@$(MKDIRHIER) $(DOCTOKHXXOUTDIR)
	@$(TOUCH) $@

$(DOCTOKRESOURCEIDSHXX): $(DOCTOKHXXOUTDIRCREATED) $(DOCTOKRESOURCETOOLS) $(DOCTOKRESOURCEIDSXSL) $(DOCTOKMODEL)
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) $(DOCTOKRESOURCEIDSXSL:s!\!/!) $(DOCTOKMODEL) > $@

$(OOXMLHXXOUTDIRCREATED):
	@$(MKDIRHIER) $(OOXMLHXXOUTDIR)
	@$(TOUCH) $@

$(OOXMLPREPROCESSXSLCOPIED): $(OOXMLPREPROCESSXSL)
	@$(COPY) $(OOXMLPREPROCESSXSL) $@

$(NSXSL) : $(OOXMLMODEL) $(NAMESPACESTXT) $(NSPROCESS)
	@$(PERL) $(NSPROCESS) $(NAMESPACESTXT) > $@

$(MODELPROCESSED): $(NSXSL) $(OOXMLPREPROCESSXSLCOPIED) $(OOXMLMODEL)
	@echo "Making:   " $(@:f)
	$(COMMAND_ECHO)$(XSLTPROC) $(NSXSL) $(OOXMLMODEL) > $@

$(OOXMLRESOURCEIDSHXX): $(OOXMLHXXOUTDIRCREATED) $(OOXMLFACTORYTOOLSXSL) $(OOXMLRESOURCEIDSXSL) $(MODELPROCESSED)
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) $(OOXMLRESOURCEIDSXSL:s!\!/!) $(MODELPROCESSED) > $@

$(SPRMIDSHXX): $(DOCTOKHXXOUTDIRCREATED) $(DOCTOKSPRMIDSXSL) $(DOCTOKMODEL)
    @echo "Making:   " $(@:f)   
	$(COMMAND_ECHO)$(XSLTPROC) $(DOCTOKSPRMIDSXSL:s!\!/!) $(DOCTOKMODEL) > $@

.PHONY: genclean genmake gendirs

genclean: 
	rm -f $(GENERATEDFILES)

genmake: $(GENERATEDFILES)

